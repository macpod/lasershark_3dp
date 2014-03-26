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

#include "LaserSharkJSONServer.h"
#include <jsonrpc/rpc.h>
#include <iostream>

#include "base64/base64_cpp.h"
#include "AbstractLaserSharkLayer.h"
#include "LaserSharkZigZagLayer.h"
#include "debug.h"

const std::string LaserSharkJSONServer::LASERSHARK_JSON_SERVER_VERSION = "1";



LaserSharkJSONServer::LaserSharkJSONServer() :
	AbstractLaserSharkJSONServer(new jsonrpc::HttpServer(8080))
{
	lasershark = NULL;
}


bool LaserSharkJSONServer::setLaserShark(LaserShark *lasershark)
{
	if (this->lasershark) {
		return false;
	}
	this->lasershark = lasershark;

	return true;
}


std::string LaserSharkJSONServer::getLaserSharkJSONVersion()
{
	return LASERSHARK_JSON_SERVER_VERSION;
}


Json::Value LaserSharkJSONServer::getLayerDone()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	ret["value"] = lasershark->layerDone();

	return ret;
}


Json::Value LaserSharkJSONServer::getLayerErrorMessage()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	ret["value"] = getLayerErrorMessage();

	return ret;
}

Json::Value LaserSharkJSONServer::getLayerRunning()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	ret["value"] = lasershark->layerRunning();

	return ret;
}


Json::Value LaserSharkJSONServer::getLayerSamplesLeft()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	ret["value"] = lasershark->getLayerSamplesLeft();

	return ret;
}


Json::Value LaserSharkJSONServer::getLayerTotalSamples()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;

	}
	ret["value"] = lasershark->getLayerTotalSamples();

	return ret;
}


Json::Value LaserSharkJSONServer::getMaxSampleRate()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	try {
		ret["value"] = lasershark->getMaxSampleRate();
	} catch (std::runtime_error e) {
        prepForFailure(ret, e.what());
        return ret;
    }

	return ret;
}


Json::Value LaserSharkJSONServer::getResolution()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	try {
		ret["value"] = lasershark->getResolution();
	} catch (std::runtime_error e) {
        prepForFailure(ret, e.what());
        return ret;
    }

	return ret;
}


void LaserSharkJSONServer::printText(const std::string& text)
{
	std::cout << text <<std::endl;
}


Json::Value LaserSharkJSONServer::sendLayer(const std::string& base64PNGData, const int& xUpperLeftPos, const int& yUpperLeftPos)
{
	Json::Value ret;
	prepForSuccess(ret);

	//std::cout << "encoded image len: " << base64PNGData.length() << " xOrg: " << xUpperLeftPos << " yOrg: " << yUpperLeftPos << std::endl;

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

    int decoded_size = base64::base64_decoded_size(base64PNGData.length());
	if (!decoded_size) {
		prepForFailure(ret, "Base64 decode size was zero.");
		return ret;
	} 

    unsigned char *decoded_image = (unsigned char*)base64::base64_decode(base64PNGData.c_str());
    D(std::cout << "Layer decode size " <<  decoded_size << std::endl;)
    if (!decoded_image) {
		prepForFailure(ret, "Base64 decoding failed.");
		return ret;
    }
	
	AbstractLaserSharkLayer *layer = new LaserSharkZigZagLayer();
	if (!layer) {
		prepForFailure(ret, "Could not allocate layer.");
		return ret;
	}

	if (!layer->populate(xUpperLeftPos, yUpperLeftPos, decoded_image, decoded_size)) {
		prepForFailure(ret, "LaserShark layer did not populate.");
		return ret;	
	}

	if (!lasershark->setLayer(layer)) {
		prepForFailure(ret, "LaserShark rejected layer. Is a layer running?");
		return ret;
	}

	return ret;
}


Json::Value LaserSharkJSONServer::setSampleRate(const int& rate)
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}

	try {
		if (!lasershark->setSampleRate(rate)) {
			prepForFailure(ret, "LaserShark could not set sample rate.");
			return ret;
		}
	} catch (std::runtime_error e) {
        prepForFailure(ret, e.what());
        return ret;
    }

	return ret;
}


Json::Value LaserSharkJSONServer::startLayer()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}


	try {
		if (!lasershark->startLayer()) {
			prepForFailure(ret, "LaserShark could not start layer.");
			return ret;
		}
	} catch (std::runtime_error e) {
        prepForFailure(ret, e.what());
        return ret;
    }

	return ret;
}


Json::Value LaserSharkJSONServer::stopAndClearLayer()
{
	Json::Value ret;
	prepForSuccess(ret);

	if (!checkLaserSharkInitialization(ret)) {
		return ret;
	}
	
	lasershark->stopAndClearLayer();	

	return ret;
}


void LaserSharkJSONServer::prepForSuccess(Json::Value &obj)
{
	obj["success"] = true;
	obj["message"] = "Success";
}


void LaserSharkJSONServer::prepForFailure(Json::Value &obj, std::string message)
{
		obj["success"] = false;
		obj["message"] = message;
        std::cerr << obj["message"] << std::endl;
}


bool LaserSharkJSONServer::checkLaserSharkInitialization(Json::Value &obj)
{
	if (!lasershark) {
		prepForFailure(obj, "LaserShark not initialized.");
		return false;
	}
	
	return true;
}




