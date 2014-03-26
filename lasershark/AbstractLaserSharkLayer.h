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


#ifndef _ABSTRACTLASERSHARKLAYER_H_
#define _ABSTRACTLASERSHARKLAYER_H_


// Not intended to be thread safe.
class AbstractLaserSharkLayer
{
	public:
		virtual ~AbstractLaserSharkLayer() = 0;

		virtual bool populate(unsigned int x_origin, unsigned int y_origin, 
			const unsigned char* png_image_data, unsigned int png_image_data_len) = 0;
		virtual void clear() = 0;
		virtual bool populated() = 0;

		virtual unsigned int fillLaserSharkTransferBuffer(unsigned int sample_count, 
			unsigned char *buf) = 0;
		virtual unsigned int getSamplesLeft() = 0;
		virtual unsigned int getTotalSamples() = 0;
		virtual unsigned int getWidth() = 0;
		virtual unsigned int getHeight() = 0;
};

inline AbstractLaserSharkLayer::~AbstractLaserSharkLayer() { }

#endif //_ABSTRACTLASERSHARKLAYER_H_

