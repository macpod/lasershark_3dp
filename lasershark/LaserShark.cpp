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

#include "LaserShark.h"

#include <sstream>
#include <thread>
#include <atomic>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include "debug.h"

#define LASERSHARK_VIN 0x1fc9
#define LASERSHARK_PID 0x04d8

#define LASERSHARK_CMD_SUCCESS 0x00
#define LASERSHARK_CMD_FAIL 0x01
#define LASERSHARK_CMD_UNKNOWN 0xFF

#define LASERSHARK_SAMPLE_SIZE 8
#define LASERSHARK_SAMPLE_COUNT_PER_BULK_TRANSFER 64

#define LASERSHARK_DEFAULT_SAMPLE_RATE 20000

// Set output commands
#define LASERSHARK_CMD_SET_OUTPUT 0x80
#define LASERSHARK_CMD_OUTPUT_ENABLE 0x01
#define LASERSHARK_CMD_OUTPUT_DISABLE 0x00

// Set/get current ilda rate
#define LASERSHARK_CMD_SET_ILDA_RATE 0x82
#define LASERSHARK_CMD_GET_ILDA_RATE 0x83

// Get max ilda rate
#define LASERSHARK_CMD_GET_MAX_ILDA_RATE 0X84

// Get max dac value
#define LASERSHARK_CMD_GET_DAC_MAX 0x88

// Get the number of samples the ring buffer is able to store
#define LASERSHARK_CMD_GET_RINGBUFFER_SAMPLE_COUNT 0X89

// Get the number of samples that are unfilled in the ring buffer
#define LASERSHARK_CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT 0X8A

// Version Info
#define LASERSHARK_FW_MAJOR_VERSION 2
#define LASERSHARK_FW_MINOR_VERSION 2 
#define LASERSHARK_CMD_GET_LASERSHARK_FW_MAJOR_VERSION 0X8B
#define LASERSHARK_GMD_GET_LASERSHARK_FW_MINOR_VERSION 0X8C

// Clears ringbuffer
#define LASERSHARK_CMD_CLEAR_RINGBUFFER 0x8D


LaserShark::LaserShark()
{
	devh_ctl = NULL;
	devh_data = NULL;
	devh_ctl_claimed = false;
	devh_data_claimed = false;
	thread_should_run = false;
	thread_running = false;
	push_thread = NULL;
	layer = NULL;
}

LaserShark::~LaserShark()
{
	if (connected()) {
		disconnect();
	}
}

/*
	Returns true if connected, false if a device was not found or already connected
	Throws errors if an issue is encountered while connecting.
	libusb is expected to have been initialized before calling this.
*/
bool LaserShark::connect() throw (std::runtime_error)
{
	int rc;
	cmd_mutex.lock();
	if (connected()) {
		// Already connected.
		cmd_mutex.unlock();
		return false;
	}


    devh_ctl = libusb_open_device_with_vid_pid(NULL, LASERSHARK_VIN, LASERSHARK_PID);
    devh_data = libusb_open_device_with_vid_pid(NULL, LASERSHARK_VIN, LASERSHARK_PID);
    if (!devh_ctl || !devh_data)
    {
		// No device
		shutdown();
		cmd_mutex.unlock();

		return false;
    }


    libusb_set_debug(NULL, 3);

    rc = libusb_claim_interface(devh_ctl, 0);
    if (rc < 0)
    {
		shutdown();
		cmd_mutex.unlock();

		std::ostringstream oss;
		oss << "Error claiming control interface: " << libusb_error_name(rc);
		std::cerr << oss << std::endl;
    	throw std::runtime_error(oss.str()); 
    }
	devh_ctl_claimed = true;


    rc = libusb_claim_interface(devh_data, 1);
    if (rc < 0)
    {
		release();
		shutdown();
		cmd_mutex.unlock();

		std::ostringstream oss;
		oss << "Error claiming data interface: " << libusb_error_name(rc);
    	throw std::runtime_error(oss.str());
    }
	devh_data_claimed = true;


    // Use the following if you want to use BULK transfers instead of ISO transfers in your hostapp code.
    rc = libusb_set_interface_alt_setting(devh_data, 1, 1);
    if (rc < 0)
    {
		release();
		shutdown();
		cmd_mutex.unlock();

		std::ostringstream oss;
		oss << "Error setting alternative (BULK) data interface: " << libusb_error_name(rc);
    	throw std::runtime_error(oss.str()); 
    }

    cmd_mutex.unlock();
	
	try {
		setOutput(false);

		// Set the default sample rate.
		setSampleRate(LASERSHARK_DEFAULT_SAMPLE_RATE);
	} catch (std::runtime_error e) {
			disconnect(); 
			std::ostringstream oss;
			oss << "Error connecting: " << e.what();
	    	throw std::runtime_error(oss.str());
	}

    return true;
}

bool LaserShark::connected()
{
	return devh_ctl_claimed;
}


void LaserShark::disconnect() 
{
	stopAndClearLayer();
	cmd_mutex.lock();
	release();
	shutdown();
	cmd_mutex.unlock();

}

/*
	Returns false if unconnected, a lasershark protocol failure occured, or true if set.
	Throws an error on transport faults.
*/

bool LaserShark::setSampleRate(unsigned int rate) throw (std::runtime_error)
{
	return setUint32(LASERSHARK_CMD_SET_ILDA_RATE, rate);
}


/*
	Returns 0 if unconnected, a lasershark protocol failure occured, or the max sample rate.
	Throws an error on transport faults.
*/
unsigned int LaserShark::getMaxSampleRate() throw (std::runtime_error)
{
	unsigned int tmp;	
	return getUint32(LASERSHARK_CMD_GET_ILDA_RATE, &tmp) ? tmp : 0; 
}


/*
	Returns 0 if unconnected, a lasershark protocol failure occured, or the resolution.
	Throws an error on transport faults.
*/
unsigned int LaserShark::getResolution()  throw (std::runtime_error)
{
	unsigned int tmp;
	return getUint32(LASERSHARK_CMD_GET_DAC_MAX, &tmp) ? tmp : 0;
}


unsigned int LaserShark::getFWMajorVersion() throw (std::runtime_error)
{
	unsigned int tmp;
	return getUint32(LASERSHARK_CMD_GET_DAC_MAX, &tmp) ? tmp : 0;
}


unsigned int LaserShark::getFWMinorVersion() throw (std::runtime_error)
{
	unsigned int tmp;
	return getUint32(LASERSHARK_CMD_GET_DAC_MAX, &tmp) ? tmp : 0;
}


/*
	Returns true if layer was set. Returns false if layer could not be set because passed in layer
	was null or because layer is currently running.
*/
bool LaserShark::setLayer(AbstractLaserSharkLayer *layer)
{
	bool res = false;

	push_thread_mutex.lock();
	layer_mutex.lock();	
	if (layer && !layerRunning()) {
		if (this->layer) {
			delete this->layer;
		}
		this->layer = layer;
		res = true;
	}
	layer_mutex.unlock();
	push_thread_mutex.unlock();
	
	return res;
}


/*
	If a layer thread is not running and a layer is defined, a new thread is started and true is returned.
	false is returned otherwise.
*/

bool LaserShark::startLayer() throw (std::runtime_error)
{
	bool res = false;

	push_thread_mutex.lock();
	if (!layerRunning()) {
		cleanupPushThread();
		if (layer) {
			thread_should_run = true;
			thread_running = true;	
			push_thread = new std::thread(&LaserShark::pushLayerThread, this); 	
			if (!push_thread) { // This should throw something
				push_thread_mutex.unlock();
				std::ostringstream oss;
				oss << "Error allocating thread.";
    			throw std::runtime_error(oss.str()); 
			}				
			res = true;
		}	
	}		
	push_thread_mutex.unlock();

	return res;
}


/*
	Stops the thread if running, then deletes it and the layer.
*/
void LaserShark::stopAndClearLayer()
{
	push_thread_mutex.lock();
	cleanupPushThread();
	layer_mutex.lock();
	push_thread_mutex.unlock();
	cleanupLayer();
	layer_mutex.unlock();
}



unsigned int LaserShark::getLayerTotalSamples()
{
	unsigned int res = 0;
	push_thread_mutex.lock();
	if (layer) {
		res = layer->getTotalSamples();
	}
	push_thread_mutex.unlock();
	return res;
}

unsigned int LaserShark::getLayerSamplesLeft()
{
	unsigned int res = 0;
	push_thread_mutex.lock();
	if (layer) {
		res = layer->getSamplesLeft();
	}
	push_thread_mutex.unlock();
	return res;
}


bool LaserShark::layerRunning()
{
	return thread_running;
}

bool LaserShark::layerDone()
{
	return !layerRunning();
}

std::string LaserShark::getLayerErrorMessage()
{
	push_thread_mutex.lock();
	std::string ret = layer_error_message;
	push_thread_mutex.unlock();
	return ret;
}


void LaserShark::release()
{
	if (devh_ctl_claimed) {
		devh_ctl_claimed = false;
    	libusb_release_interface(devh_ctl, 0);
	}
	if (devh_ctl_claimed) {
		devh_data_claimed = false; 
 		libusb_release_interface(devh_data, 0);
	}
}


void LaserShark::shutdown()
{
    if (devh_ctl)
    {
        libusb_close(devh_ctl);
		devh_ctl = NULL;
    }
    if (devh_data)
    {
        libusb_close(devh_data);
		devh_data = NULL;
    }
}



void LaserShark::cleanupPushThread()
{
	if (push_thread) {
		thread_should_run = false;
		push_thread->join();
		delete push_thread;
		push_thread = NULL;
		thread_running = false;
		layer_error_message.clear();
	}
}


void LaserShark::cleanupLayer()
{
	if (layer) {
		delete layer;
		layer = NULL;
	}
}

#include <unistd.h>

void LaserShark::pushLayerThread()
{
	D(std::cout << "^LS thread starting" << std::endl;)

	unsigned int samples_to_send = LASERSHARK_SAMPLE_COUNT_PER_BULK_TRANSFER;
	char *buf = new char[LASERSHARK_SAMPLE_COUNT_PER_BULK_TRANSFER*LASERSHARK_SAMPLE_SIZE];
	if (!buf) {
		layer_error_message = "Could not allocate transfer buffer";
		D(std::cout << layer_error_message << std::endl;)
		thread_should_run = false;
		thread_running = false;
		return;
	}
	memset(buf, 0, LASERSHARK_SAMPLE_COUNT_PER_BULK_TRANSFER*LASERSHARK_SAMPLE_SIZE);

	unsigned int ringbuffer_samples = 0;
	try {
		ringbuffer_samples = getRingbufferSampleCount();
		if (!ringbuffer_samples) {
			std::ostringstream oss;
			oss << "Error getting ringbuffer samples.";
	    	throw std::runtime_error(oss.str());
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		layer_error_message = e.what();
		thread_should_run = false;
		push_thread_mutex.unlock();
	}

	// Check resolution of layer
	unsigned int resolution = 0;
	try {	
		resolution = getResolution();
		if (resolution == 0) {
			std::ostringstream oss;
			oss << "Lasershark resolution was reported as 0";
			throw std::runtime_error(oss.str());
		}
		if (layer->getWidth() > resolution || layer->getHeight() > resolution) {
			std::ostringstream oss;
			std::cerr << "Layer width or high exceeded resolution" << resolution << std::endl;
			throw std::runtime_error(oss.str());
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		layer_error_message = e.what();
		thread_should_run = false;
		push_thread_mutex.unlock();
	}


	// Clear the ringbuffer
	try {
		if (!clearSamples()) {
			std::ostringstream oss;
			oss << "Error clearing ringbuffer.";
	    	throw std::runtime_error(oss.str());
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		layer_error_message = e.what();
		thread_should_run = false;
		push_thread_mutex.unlock();
	}

	// Enable the output
	try {
		if (!setOutput(true)) {
			std::ostringstream oss;
			oss << "Error enabling output.";
	    	throw std::runtime_error(oss.str());
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		layer_error_message = e.what();
		thread_should_run = false;
		push_thread_mutex.unlock();
	}


	if (thread_should_run) {
		while (thread_should_run) {
			if (layer->getSamplesLeft() < samples_to_send) {
				samples_to_send = layer->getSamplesLeft();
			}

			if (samples_to_send == 0) {
				break;
			} else {
				try {
					packAndSendSamples(samples_to_send, buf);
				} catch (std::runtime_error e) {
					push_thread_mutex.lock();
					layer_error_message = e.what();
					thread_should_run = false;
					push_thread_mutex.unlock();
				}
 			}
		}
	}


	// Wait for all samples to complete before stopping (assuming nobody instructed us to quit).
	// If we don't do this not all samples may be printed!
	try {
		
		while (thread_should_run && getRingbufferEmptySampleCount() != ringbuffer_samples - 1) {
			asm("nop");
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		if (layer_error_message.length() == 0) {
			layer_error_message = e.what();
		} else {
			layer_error_message += ". Also could not wait for all samples to be completed.";
		}
		push_thread_mutex.unlock();
	}


	// Disable the output
	try {
		if (!setOutput(false)) {
			std::ostringstream oss;
			oss << "Error disabling output.";
	    	throw std::runtime_error(oss.str());
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		if (layer_error_message.length() == 0) {
			layer_error_message = e.what();
		} else {
			layer_error_message += ". Also could not disable output.";
		}
		push_thread_mutex.unlock();
	}

	// Clear out samples that may still be in the buffer (could occur if someone instructed us to quit).
	try {
		if (!clearSamples()) {
			std::ostringstream oss;
			oss << "Error disabling output.";
	    	throw std::runtime_error(oss.str());
		}
	} catch (std::runtime_error e) {
		push_thread_mutex.lock();
		if (layer_error_message.length() == 0) {
			layer_error_message = e.what();
		} else {
			layer_error_message += ". Also could not clear layer.";
		}
		push_thread_mutex.unlock();
	}


	delete[] buf;

	layer_mutex.lock();
	delete layer;
	layer = NULL;
	layer_mutex.unlock();

	thread_should_run = false;
	thread_running = false;
	D(std::cout << "^LS thread exiting" << std::endl;)
}

void LaserShark::packAndSendSamples(unsigned int to_send_samples, char *buf)  throw (std::runtime_error)
{
	int r = 0, actual;
	unsigned int len = to_send_samples * LASERSHARK_SAMPLE_SIZE;

	//D(std::cout << "+sending " << to_send_samples << " left: " << layer->getSamplesLeft() << std::endl;)

	layer->fillLaserSharkTransferBuffer(to_send_samples, (unsigned char*)buf);

    r = libusb_bulk_transfer(devh_data, (3 | LIBUSB_ENDPOINT_OUT), (unsigned char*)buf, len, &actual, 0); // The timeout should be changed.
	if (r < 0) {
		std::ostringstream oss;
		oss << "Error disabling output: " << libusb_error_name(r);
	    throw std::runtime_error(oss.str());
	}
}


bool LaserShark::setOutput(bool enable)  throw (std::runtime_error)
{
    return setUint8(LASERSHARK_CMD_SET_OUTPUT, enable ? LASERSHARK_CMD_OUTPUT_ENABLE : LASERSHARK_CMD_OUTPUT_DISABLE);
}


bool LaserShark::clearSamples() throw (std::runtime_error)
{
    return setUint8(LASERSHARK_CMD_CLEAR_RINGBUFFER, 0); // Value sent doesn't matter.	
}


unsigned int LaserShark::getRingbufferSampleCount() throw (std::runtime_error)
{
	unsigned int tmp;
	return getUint32(LASERSHARK_CMD_GET_RINGBUFFER_SAMPLE_COUNT, &tmp) ? tmp : 0;
}


unsigned int LaserShark::getRingbufferEmptySampleCount() throw (std::runtime_error)
{
	unsigned int tmp;
	return getUint32(LASERSHARK_CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT, &tmp) ? tmp : 0;
}


bool LaserShark::getUint32(uint8_t command, unsigned int *val) throw (std::runtime_error)
{
    unsigned char data[64];
    int r, actual;
    int len = 1;

	if (!connected()) {
		return false;
	}

    data[0] = command;

	cmd_mutex.lock();	
    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len) {
		cmd_mutex.unlock();
		std::ostringstream oss;
		oss << "Error transmitting: " << libusb_error_name(r);
    	throw std::runtime_error(oss.str()); 
	}

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

	cmd_mutex.unlock();

    if(r != 0 || actual != 64) {
		std::ostringstream oss;
		oss << "Error receiving: " << libusb_error_name(r);
    	throw std::runtime_error(oss.str()); 
	}

	if (data[1] != LASERSHARK_CMD_SUCCESS) {
        //std::cout << "Read Error: " << libusb_error_name(r) << " " << actual << std::endl;
        return false;
    }
    memcpy(val, data + 2, sizeof(uint32_t));

    return true;
}


bool LaserShark::setUint8(unsigned char command, unsigned char val) throw (std::runtime_error)
{
    unsigned char data[64];
    int r, actual;
    int len = 2;

	if (!connected()) {
		return false;
	}


    data[0] = command;
    data[1] = val;


	cmd_mutex.lock();

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len) {
		cmd_mutex.unlock();
		std::ostringstream oss;
		oss << "Error transmitting: " << libusb_error_name(r);
    	throw std::runtime_error(oss.str()); 
	}

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

	cmd_mutex.unlock();

    if(r != 0 || actual != 64) {
		std::ostringstream oss;
		oss << "Error receiving: " << libusb_error_name(r);
    	throw std::runtime_error(oss.str()); 
	}

	if (data[1] != LASERSHARK_CMD_SUCCESS) {
        //std::cout << "Read Error: " << libusb_error_name(r) << " " << actual << std::endl;
        return false;
    }

    return true;

}


bool LaserShark::setUint32(unsigned char command, unsigned int val) throw (std::runtime_error)
{
    unsigned char data[64];
    int r, actual;
    int len = 1 + sizeof(uint32_t);

	if (!connected()) {
		return false;
	}

    data[0] = command;
    memcpy(data + 1, &val, sizeof(uint32_t));

	cmd_mutex.lock();
    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len) {
		cmd_mutex.unlock();
		std::ostringstream oss;
		oss << "Error transmitting: " << libusb_error_name(r);
    	throw std::runtime_error(oss.str()); 
	}

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

	cmd_mutex.unlock();

    if(r != 0 || actual != 64) {
		std::ostringstream oss;
		oss << "Error receiving: " << libusb_error_name(r);
    	throw std::runtime_error(oss.str()); 
	}

	if(data[1] != LASERSHARK_CMD_SUCCESS) {
        //std::cout << "Read Error: " << libusb_error_name(r) << " " << actual << std::endl;
        return false;
    }

    return true;
}

