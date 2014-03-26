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

#ifndef _LASERSHARKZIGZAGLAYER_H_
#define _LASERSHARKZIGZAGLAYER_H_

#include "AbstractLaserSharkLayer.h"
#include <vector>


// Not intended to be thread safe.
class LaserSharkZigZagLayer : public AbstractLaserSharkLayer
{
	public:
		LaserSharkZigZagLayer();
		bool populate(unsigned int x_origin, unsigned int y_origin, const unsigned char* png_image_data, unsigned int png_image_data_len);
		void clear();
		bool populated();

		unsigned int fillLaserSharkTransferBuffer(unsigned int sample_count, unsigned char *buf);
		unsigned int getSamplesLeft();
		unsigned int getTotalSamples();
		unsigned int getWidth();
		unsigned int getHeight();


	private:
		bool initialized;
		unsigned int width, height;
		unsigned int x_origin, y_origin;
		std::vector<unsigned char> image;
		unsigned int curr_x_pos, curr_y_pos;
		unsigned int total_samples, samples_left;

};

#endif //_LASERSHARKLAYER_H_

