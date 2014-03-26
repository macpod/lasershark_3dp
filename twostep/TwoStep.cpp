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

#include <iostream>
#include <sstream>
#include "TwoStep.h"
#include "debug.h"

extern "C" {
#include "lasershark_hostapp/lasershark_uart_bridge_lib.h"
#include "lasershark_hostapp/ls_ub_twostep_lib.h"
}


#define LASERSHARK_VIN 0x1fc9
#define LASERSHARK_PID 0x04d8

//#define TWOSTEP_VERSION;


TwoStep::TwoStep()
{
	devh_ub = NULL;
	devh_ub_claimed = false;
}

TwoStep::~TwoStep()
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
bool TwoStep::connect() throw (std::runtime_error)
{
	int rc;
	ub_mutex.lock();
	if (connected()) {
		// Already connected.
		ub_mutex.unlock();
		return false;
	}


    devh_ub = libusb_open_device_with_vid_pid(NULL, LASERSHARK_VIN, LASERSHARK_PID);
    if (!devh_ub)
    {
		// No device
		shutdown();
		ub_mutex.unlock();

		return false;
    }

    rc = libusb_claim_interface(devh_ub, 2);
    if (rc < 0)
    {
		shutdown();
		ub_mutex.unlock();

		std::ostringstream oss;
		oss << "Error claiming control interface: " << libusb_error_name(rc);
		std::cerr << oss << std::endl;
    	throw std::runtime_error(oss.str()); 
    }
	devh_ub_claimed = true;


    ub_mutex.unlock();
	
	try { 
		D(int version = getVersion();)
		D(std::cout << "version was: " << version << std::endl;)

		stop(true, true);
		setEnable(TWOSTEP_STEPPER_1, false);
		setEnable(TWOSTEP_STEPPER_2, false);

		setMicrosteps(TWOSTEP_STEPPER_1, TWOSTEP_MICROSTEP_BITFIELD_FULL_STEP);
		setMicrosteps(TWOSTEP_STEPPER_2, TWOSTEP_MICROSTEP_BITFIELD_FULL_STEP);
		setCurrent(TWOSTEP_STEPPER_1, TWOSTEP_MIN_CURRENT_VAL);
		setCurrent(TWOSTEP_STEPPER_2, TWOSTEP_MIN_CURRENT_VAL);
		set100uSDelay(TWOSTEP_STEPPER_1, TWOSTEP_STEP_100US_DELAY_5MS);

	} catch (std::runtime_error e) {
			disconnect(); 
			std::ostringstream oss;
			oss << "Error connecting: " << e.what();
	    	throw std::runtime_error(oss.str());
	}

    return true;
}


bool TwoStep::connected()
{
	return devh_ub_claimed;
}


void TwoStep::disconnect() 
{
	stopAndDisable();
	ub_mutex.lock();
	release();
	shutdown();
	ub_mutex.unlock();

}


void TwoStep::setSafeSteps(int stepperNum, int steps) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_safe_steps(devh_ub, stepperNum, steps);
	ub_mutex.unlock();

	handleBadResponse(res);
}


void TwoStep::setSteps(int stepperNum, int steps) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_steps(devh_ub, stepperNum, steps);
	ub_mutex.unlock();

	handleBadResponse(res);
}

void TwoStep::setStepUntilSwitch(int stepperNum) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_step_until_switch(devh_ub, stepperNum);
	ub_mutex.unlock();
	
	handleBadResponse(res);
}

void TwoStep::start(bool stepperOne, bool stepperTwo) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_start(devh_ub, (stepperOne ? TWOSTEP_STEPPER_BITFIELD_STEPPER_1 : 0) | (stepperTwo ? TWOSTEP_STEPPER_BITFIELD_STEPPER_2 : 0));
	ub_mutex.unlock();

	handleBadResponse(res);
}


void TwoStep::stop(bool stepperOne, bool stepperTwo) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_stop(devh_ub, (stepperOne ? TWOSTEP_STEPPER_BITFIELD_STEPPER_1 : 0) | (stepperTwo ? TWOSTEP_STEPPER_BITFIELD_STEPPER_2 : 0));
	ub_mutex.unlock();

	handleBadResponse(res);
}


bool TwoStep::getIsMoving(int stepperNum) throw (std::runtime_error)
{
	bool value;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_is_moving(devh_ub, stepperNum, &value);
	ub_mutex.unlock();

	handleBadResponse(res);
	return value;
}


void TwoStep::setEnable(int stepperNum, bool enable) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_enable(devh_ub, stepperNum, enable);
	ub_mutex.unlock();

	handleBadResponse(res);
}


bool TwoStep::getEnable(int stepperNum) throw (std::runtime_error)
{
	bool value;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_enable(devh_ub, stepperNum, &value);
	ub_mutex.unlock();

	handleBadResponse(res);
	return value;
}


void TwoStep::setMicrosteps(int stepperNum,  int microsteps) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_microsteps(devh_ub, stepperNum, microsteps);
	ub_mutex.unlock();

	handleBadResponse(res);
}


unsigned int TwoStep::getMicrosteps(int stepperNum) throw (std::runtime_error)
{
	unsigned char value;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_microsteps(devh_ub, stepperNum, &value);
	ub_mutex.unlock();

	handleBadResponse(res);
	return value;
}


void TwoStep::setDir(int stepperNum, bool high) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_dir(devh_ub, stepperNum, high);
	ub_mutex.unlock();

	handleBadResponse(res);
}


bool TwoStep::getDir(int stepperNum) throw (std::runtime_error)
{
	bool high;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_dir(devh_ub, stepperNum, &high);
	ub_mutex.unlock();

	handleBadResponse(res);
	return high;
}



void TwoStep::setCurrent(int stepperNum, int value) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_current(devh_ub, stepperNum, value);
	ub_mutex.unlock();

	handleBadResponse(res);
}


unsigned int TwoStep::getCurrent(int stepperNum) throw (std::runtime_error)
{
	unsigned short value;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_current(devh_ub, stepperNum, &value);
	ub_mutex.unlock();

	handleBadResponse(res);
	return value;
}


void TwoStep::set100uSDelay(int stepperNum, int value) throw (std::runtime_error)
{
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_set_100uS_delay(devh_ub, stepperNum, value);
	ub_mutex.unlock();

	handleBadResponse(res);
}


unsigned int TwoStep::get100uSDelay(int stepperNum) throw (std::runtime_error)
{
	unsigned short value;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_100uS_delay(devh_ub, stepperNum, &value);
	ub_mutex.unlock();

	handleBadResponse(res);
	return value;
}


void TwoStep::getSwitchStatus(bool& r1_a, bool &r1_b, bool &r2_a, bool &r2_b) throw (std::runtime_error)
{
	unsigned char switches;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_switch_status(devh_ub, &switches);
	ub_mutex.unlock();

	handleBadResponse(res);

	if (switches && TWOSTEP_SWITCHES_R1_A) {
		r1_a = true;
	} else {
		r1_a = false;
	}

	if (switches && TWOSTEP_SWITCHES_R1_B) {
		r1_b = true;
	} else {
		r1_b = false;
	}

	if (switches && TWOSTEP_SWITCHES_R2_A) {
		r2_a = true;
	} else {
		r2_a = false;
	}

	if (switches && TWOSTEP_SWITCHES_R2_B) {
		r2_b = true;
	} else {
		r2_b = false;
	}

}


int TwoStep::getVersion() throw (std::runtime_error)
{
	unsigned char version;
	ub_mutex.lock();
	unsigned char res = ls_ub_twostep_get_version(devh_ub, &version);
	ub_mutex.unlock();

	handleBadResponse(res);
	return version;
}


void TwoStep::release()
{
	if (devh_ub_claimed) {
		devh_ub_claimed = false;
    	libusb_release_interface(devh_ub, 0);
	}
}


void TwoStep::shutdown()
{
    if (devh_ub)
    {
        libusb_close(devh_ub);
		devh_ub = NULL;
    }
}


void TwoStep::stopAndDisable()
{
	try { 
		// We wouldn't want to disconnect and leave the motors running!
		stop(true, true);
		setEnable(TWOSTEP_STEPPER_1, false);
		setEnable(TWOSTEP_STEPPER_2, false);

	} catch (std::runtime_error e) {
		std::cerr << "Error encountered" << e.what();
	}	
}


void TwoStep::handleBadResponse(unsigned char res) throw (std::runtime_error)
{
	std::ostringstream oss;
	bool fail = false;
	if (res == LS_UB_TWOSTEP_UB_FAIL) {
		oss << "Uart bridge general failure.";
		fail = true;
	} else if (res == LS_UB_TWOSTEP_TX_FAIL) {
		oss << "Uart bridge TX failure.";
		fail = true;
	} else if (res == LS_UB_TWOSTEP_RX_FAIL) {
		oss << "Uart bridge RX failure.";
		fail = true;
	} else if (res == LS_UB_TWOSTEP_TS_PROTO_FAIL) {
		oss << "TwoStep protocol failure.";
		fail = true;
	} else if (res == LS_UB_TWOSTEP_TS_CMD_FAIL) {
		oss << "TwoStep command failure.";
		fail = true;
	}

	if (fail) {
		throw std::runtime_error(oss.str());
	}
}


