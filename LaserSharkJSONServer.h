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

#ifndef _LASERSHARKJSONSERVER_H_
#define _LASERSHARKJSONSERVER_H_

#include "abstractlasersharkjsonserver.h"
#include "LaserShark.h"


class LaserSharkJSONServer : public AbstractLaserSharkJSONServer
{
	public:
		LaserSharkJSONServer();

		static const std::string LASERSHARK_JSON_SERVER_VERSION;

		bool setLaserShark(LaserShark *laserShark);
		
		virtual std::string getLaserSharkJSONVersion();
        virtual Json::Value getLayerDone();
        virtual Json::Value getLayerErrorMessage();
        virtual Json::Value getLayerRunning();
        virtual Json::Value getLayerSamplesLeft();
        virtual Json::Value getLayerTotalSamples();
        virtual Json::Value getMaxSampleRate();
        virtual Json::Value getResolution();
        virtual void printText(const std::string& text);
        virtual Json::Value sendLayer(const std::string& base64PNGData, 
			const int& xUpperLeftPos, const int& yUpperLeftPos);
        virtual Json::Value setSampleRate(const int& rate);
        virtual Json::Value startLayer();
        virtual Json::Value stopAndClearLayer();

	private: 
		LaserShark *lasershark;

		void prepForSuccess(Json::Value &obj);
		void prepForFailure(Json::Value &obj, std::string message);
		bool checkLaserSharkInitialization(Json::Value &obj);
};

#endif //_LASERSHARKJSONSERVER_H_
