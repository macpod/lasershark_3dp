/*
This file is part of the LaserShark 3d Printer host application.

Copyright (C) 2014 Jeffrey Nelson <nelsonjm@macpod.net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "LaserSharkZigZagLayer.h"
#include "lodepng.h"
#include <iostream>
#include "debug.h"

/*
todo: Improve memory efficiency
	bit depth option?
	figure out what to do about consts.
	account for oversized images (seems using throws would be best)
*/

LaserSharkZigZagLayer::LaserSharkZigZagLayer()
{
	clear();
}

bool LaserSharkZigZagLayer::populate(unsigned int x_origin, unsigned int y_origin, const unsigned char* png_image_data, unsigned int png_image_data_len)
{

	if (initialized) {
	    std::cerr << "Layer is populated, can't re-populate." << std::endl;
		return false;
	}


	// TODO allow option to not use 16 bits.
    unsigned error = lodepng::decode(image, width, height, png_image_data, png_image_data_len, LCT_GREY, 8); 

    //if there's an error, display it
    if(error) {
		std::cerr << "Layer decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		return false;
	}


    D(std::cout << "Layer dimensions x: " << width << " y: " << height << std::endl;)

	total_samples = width*height;//image.size();

	if (total_samples == 0) {
		std::cerr << "Layer was empty!" << std::endl;
		clear();
		return false;
	}


	initialized = true;
	curr_x_pos = 0;
	curr_y_pos = 0;

	for (unsigned int i = total_samples; i > 0; i--) {
			if (0 == image[i-1]) {
				total_samples--;
			}
	}

    D(std::cout << "Layer total_samples:" << total_samples  << std::endl;)

	samples_left = total_samples;
	
/*
	std::cout << "Image:" << std::endl;
	unsigned int x, y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (image[y*width + x] > 255/2) {
				std::cout << "o";
			} else {
				std::cout << ".";
			}
		}
		std::cout << std::endl;
	}
	*/

	return true;
}


/*
This function is only compatible with Lasershark V2.X modules. The format is a 16 byte (little endian) array of 4 elements
[0] = Channel A output (lower 12 bits), LASERSHARK_C_BITMASK field(0x4000), LASERSHARK_INTL_A_BITMASK(0x8000)
[1] = Channel B output (lower 12 bits)
[2] = X Galvo output (lower 12 bits)
[3] = Y Galvo output (lower 12 bits).

It's pretty messy... A version command should probably be added to the protocol and a check added so this can be used
with multiple different lasershark versions... but for now, it's good enough.

*/
unsigned int LaserSharkZigZagLayer::fillLaserSharkTransferBuffer(unsigned int sample_count, unsigned char *buf)
{
	if (!initialized) {
		return 0;
	}

	if (buf == NULL) {
		std::cerr << "Layer fillLaserSharkTransferBuffer buff was null!" << std::endl;
		return 0; // TODO throw error?
	}

	struct laserSharkSample
	{
	    unsigned short a	: 12;
		unsigned short pad	: 2;
		bool c				: 1;
		bool intl_a			: 1; 
	    unsigned short b	: 16;
	    unsigned short x	: 16;
	    unsigned short y	: 16;
	} __attribute__((packed)) *sample = (laserSharkSample*)buf;

    // This is being removed.. we send "blank" pixels and that could definitely be more than samples_left
	/*if (sample_count > samples_left) {
		sample_count = samples_left;
	}*/

	if (sample_count == 0) {
		return 0;
	}

	unsigned int count = 0;
	

	while (count < sample_count) {
        int val = (image[curr_y_pos*width + curr_x_pos] << 4); // Change to 12 bit TODO (remove divide by 8)
		sample[count].a = val;
		sample[count].c = val > 2048 ? true : false; 
		sample[count].intl_a = true; 
		sample[count].b = val;
		sample[count].x = curr_x_pos + x_origin;
		sample[count].y = curr_y_pos + y_origin;
		count++;
		
		if (val) {
		    samples_left--;
		}

        printf("\tx:\t%d\ty:\t%d\t= %d\n", curr_y_pos, curr_x_pos, val);


        if (curr_y_pos & 1) { // Odd row
			if (curr_x_pos == 0) {
			    curr_y_pos++;
            } else {
                curr_x_pos--;
            }        
        } else { // Even row
            if (curr_x_pos == width-1) {
                curr_y_pos++;
            } else {
                curr_x_pos++;
            }
        }
	}
	
	D(std::cout << "sl: " << samples_left << " y: " << curr_y_pos << " x: " << curr_x_pos << std::endl;)

	return count;
	
}


unsigned int LaserSharkZigZagLayer::getSamplesLeft()
{
	return samples_left;
}


unsigned int LaserSharkZigZagLayer::getTotalSamples()
{
	return total_samples;
}

unsigned int LaserSharkZigZagLayer::getWidth()
{
	return width;
}
unsigned int LaserSharkZigZagLayer::getHeight()
{
	return height;
}



void LaserSharkZigZagLayer::clear()
{
	image.clear();
	width = 0;
	height = 0;
	x_origin = 0;
	y_origin = 0;
	curr_x_pos = 0;
	curr_y_pos = 0;
	total_samples = 0;
	samples_left = 0;
	initialized = false;
}


bool LaserSharkZigZagLayer::populated()
{
	return initialized;
}

