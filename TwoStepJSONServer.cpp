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

#include "TwoStepJSONServer.h"
#include <jsonrpc/rpc.h>

const std::string TwoStepJSONServer::TWOSTEP_JSON_SERVER_VERSION = "1";

TwoStepJSONServer::TwoStepJSONServer() :
	AbstractTwoStepJSONServer(new jsonrpc::HttpServer(8080))
{
//	twostep = NULL;
}



Json::Value TwoStepJSONServer::get100uSDelay(const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::getCurrent(const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::getDir(const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::getEnable(const bool& enable, const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::getIsMoving(const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::getMicrosteps(const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::getSwitchStatus()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


std::string TwoStepJSONServer::getTwoStepJSONVersion()
{
	return TWOSTEP_JSON_SERVER_VERSION;
}


Json::Value TwoStepJSONServer::getVersion()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	try {
		ret["value"] = twoStep->getVersion();
	} catch (std::runtime_error e) {
        prepForFailure(ret, e.what());
        return ret;
    }

	return ret;
}

void TwoStepJSONServer::printText(const std::string& text)
{
	std::cout << text <<std::endl;
}


Json::Value TwoStepJSONServer::set100uSDelay(const bool& enable, const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::setCurrent(const bool& enable, const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

/*
	try {
		if (!twoStep->setCurrent(rate)) {
			prepForFailure(ret, "LaserShark could not set sample rate.");
			return ret;
		}
	} catch (std::runtime_error e) {
        prepForFailure(ret, e.what());
        return ret;
    }
*/
/*
	try {
		if (!setCurrent(const bool& enable, const int& stepperNum) throw (std::runtime_error);
	}
*/
	return ret;
}


Json::Value TwoStepJSONServer::setDir(const bool& high, const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::setEnable(const bool& enable, const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::setMicrosteps(const bool& enable, const int& stepperNum)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::setSafeSteps(const int& stepperNum, const int& steps)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::setSteps(const int& stepperNum, const int& steps)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::start(const bool& stepperOne, const bool& stepperTwo)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


Json::Value TwoStepJSONServer::stop(const bool& stepperOne, const bool& stepperTwo)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkTwoStepInitialization(ret)) {
		return ret;
	}

	return ret;
}


void TwoStepJSONServer::prepForSuccess(Json::Value &obj)
{
	obj["success"] = true;
	obj["message"] = "Success";
}


void TwoStepJSONServer::prepForFailure(Json::Value &obj, std::string message)
{
		obj["success"] = false;
		obj["message"] = message;
        std::cerr << obj["message"] << std::endl;
}


bool TwoStepJSONServer::checkTwoStepInitialization(Json::Value &obj)
{
	if (!twoStep) {
		prepForFailure(obj, "TwoStep not initialized.");
		return false;
	}
	
	return true;
}


