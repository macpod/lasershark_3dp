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

#ifndef _TWOSTEP_H_
#define _TWOSTEP_H_
#include <stdexcept>
#include <mutex>

class TwoStep
{
	public:
		TwoStep();
		~TwoStep();

		bool connect() throw (std::runtime_error);
		bool connected();
		void disconnect();


        void setSafeSteps(int stepperNum, int steps) throw (std::runtime_error);
        void setSteps(int stepperNum, int steps) throw (std::runtime_error);
		void setStepUntilSwitch(int stepperNum) throw (std::runtime_error);
	
        void start(bool stepperOne, bool stepperTwo) throw (std::runtime_error);
        void stop(bool stepperOne, bool stepperTwo) throw (std::runtime_error);

        bool getIsMoving(int stepperNum) throw (std::runtime_error);

        void setEnable(int stepperNum, bool enable) throw (std::runtime_error);
        bool getEnable(const int stepperNum) throw (std::runtime_error);

        void setMicrosteps(int stepperNum, int value) throw (std::runtime_error);
        unsigned int getMicrosteps(int stepperNum) throw (std::runtime_error);

        void setDir(int stepperNum, bool high) throw (std::runtime_error);
        bool getDir(int stepperNum) throw (std::runtime_error);

        void setCurrent(int stepperNum, int value) throw (std::runtime_error);
        unsigned int getCurrent(int stepperNum) throw (std::runtime_error);

        unsigned int get100uSDelay(int stepperNum) throw (std::runtime_error);
        void set100uSDelay(int stepperNum, int value) throw (std::runtime_error);

		void getSwitchStatus(bool& r1_a, bool &r1_b, bool &r2_a, bool &r2_b) throw (std::runtime_error);

        int getVersion() throw (std::runtime_error);


	private:
		void release();
		void shutdown();

		void stopAndDisable();

		void handleBadResponse(unsigned char res) throw (std::runtime_error);

		std::mutex ub_mutex;
		bool devh_ub_claimed;
		struct libusb_device_handle *devh_ub;
};

#endif //_TWOSTEP_H_
