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

#ifndef _TWOSTEPJSONSERVER_H_
#define _TWOSTEPJSONSERVER_H_

#include "abstracttwostepjsonserver.h"
#include "TwoStep.h"

class TwoStepJSONServer : public AbstractTwoStepJSONServer
{
	public:
		TwoStepJSONServer();

		static const std::string TWOSTEP_JSON_SERVER_VERSION;

		bool setTwoStep(TwoStep *twoStep);

        virtual Json::Value get100uSDelay(const int& stepperNum);
        virtual Json::Value getCurrent(const int& stepperNum);
        virtual Json::Value getDir(const int& stepperNum);
        virtual Json::Value getEnable(const int& stepperNum);
        virtual Json::Value getIsMoving(const int& stepperNum);
        virtual Json::Value getMicrosteps(const int& stepperNum);
        virtual Json::Value getSwitchStatus();
        virtual std::string getTwoStepJSONVersion();
        virtual Json::Value getVersion();
        virtual void printText(const std::string& text);
        virtual Json::Value set100uSDelay(const int& stepperNum, const int& value);
        virtual Json::Value setCurrent(const int& stepperNum, const int& value);
        virtual Json::Value setDir(const int& stepperNum, const bool& high);
        virtual Json::Value setEnable(const int& stepperNum, const bool& enable);
        virtual Json::Value setMicrosteps(const int& stepperNum, const int& value);
        virtual Json::Value setSafeSteps(const int& stepperNum, const int& steps);
        virtual Json::Value setStepUntilSwitch(const int& stepperNum);
        virtual Json::Value setSteps(const int& stepperNum, const int& steps);
        virtual Json::Value start(const bool& stepperOne, const bool& stepperTwo);
        virtual Json::Value stop(const bool& stepperOne, const bool& stepperTwo);

	private: 
		TwoStep *twoStep;

		void prepForSuccess(Json::Value &obj);
		void prepForFailure(Json::Value &obj, std::string message);
		bool checkTwoStepInitialization(Json::Value &obj);
};

#endif //_TWOSTEPJSONSERVER_H_
