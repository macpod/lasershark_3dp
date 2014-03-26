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

#ifndef _LASERSHARK_H_
#define _LASERSHARK_H_
#include <iostream>
#include <libusb-1.0/libusb.h>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <mutex>

#include "AbstractLaserSharkLayer.h"

/*
	todo:
		allow for multiple lasershark connections
		allow for listing of lasersharks
		make singleton?
		figure out 3.0 port issue
		search for todo statements.
		write clear samples command for lasershark
*/



class LaserShark
{
	public:
		LaserShark();
		~LaserShark();

		bool connect() throw (std::runtime_error);
		bool connected();
		void disconnect();

		bool setSampleRate(unsigned int rate) throw (std::runtime_error);
		unsigned int getMaxSampleRate() throw (std::runtime_error);

		unsigned int getResolution() throw (std::runtime_error);

		unsigned int getFWMajorVersion() throw (std::runtime_error);
		unsigned int getFWMinorVersion() throw (std::runtime_error);

		bool setLayer(AbstractLaserSharkLayer *layer);

		bool startLayer()  throw (std::runtime_error);
		void stopAndClearLayer();

		unsigned int getLayerTotalSamples();
		unsigned int getLayerSamplesLeft();

		bool layerRunning();

		bool layerDone();
		std::string getLayerErrorMessage();



	private:
		void release();
		void shutdown();

		void cleanupPushThread();
		void cleanupLayer();

		void pushLayerThread();
		void packAndSendSamples(unsigned int to_send_samples, char* buf)  throw (std::runtime_error);

		bool setOutput(bool enable)  throw (std::runtime_error);
		bool clearSamples() throw (std::runtime_error);
		unsigned int getRingbufferSampleCount() throw (std::runtime_error);
		unsigned int getRingbufferEmptySampleCount() throw (std::runtime_error);

		bool setUint8(unsigned char command, unsigned char val) throw (std::runtime_error);
		bool setUint32(unsigned char command, unsigned int val) throw (std::runtime_error);
		bool getUint32(unsigned char command, unsigned int *val) throw (std::runtime_error);


		std::atomic<bool> thread_should_run;
		std::atomic<bool> thread_running;
		std::mutex push_thread_mutex;
		std::string layer_error_message;
		std::thread *push_thread;

		std::mutex layer_mutex;
		AbstractLaserSharkLayer *layer;

		std::mutex cmd_mutex;
		bool devh_ctl_claimed;
		bool devh_data_claimed;
		struct libusb_device_handle *devh_ctl;
		struct libusb_device_handle *devh_data;
};

#endif //_LASERSHARK_H_
