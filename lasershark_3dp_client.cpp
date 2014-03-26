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

#include <jsonrpc/rpc.h>
#include <iostream>
#include <fstream>

#include "base64/base64_cpp.h"
#include "lasersharkjsonclient.h"

using namespace jsonrpc;
using namespace std;


// consider max size restriction. Consider quick check to see if this file is valid.
char* read_and_base64_encode_image(char* image_file)
{
	std::ifstream file;

    file.open(image_file, std::ios::binary | std::ios::ate);

	if (file.fail()) {
        return NULL;
    }

    int file_size = file.tellg();
	if (!file_size) {
		return NULL;
	}

    char *file_data = new char[file_size];
	if (!file_data) {
		return NULL;
	}

    file.seekg(0, std::ios::beg);
    file.read(file_data, file_size);
	if (!file) {
		delete[] file_data;
		return NULL;
	}
	
	char *encoded = base64::base64_encode(file_data, file_size);
	if (!encoded) {
		delete[] file_data;
		return NULL;
	}

	

	return encoded;	
}


void cr(Json::Value value) throw (char *)
{
	if (!value["success"]) {
		throw value["message"];
	}
	
}

int main(int argc, char** argv)
{
    ifstream command_file;
	ifstream image_file;
    if (argc != 2) {
		cout << " Must specify script file" << endl;
		return 1;
    }
	
	command_file.open(argv[1]);
	if (command_file.fail()) {
		cout << "Could not open file" << endl;
		return 1;
	}

    LaserSharkJSONClient c(new HttpClient("http://localhost:8080"));
    try
    {
		//c.printText("==Homing stuff");
//		char file_name[] = "../data/5x5checkerboard.png";
//		char file_name[] = "../data/tux.png";
		char file_name[] = "../data/out_Vector_Clipart.png";
//		char file_name[] = "../data/black-and-white-halftone-background-2755.png";
//		char file_name[] = "../data/c100.png";
		char* base64_img = read_and_base64_encode_image(file_name);
		if (!base64_img) {
			throw "Could not read or encode image";
		}
        cr(c.sendLayer(base64_img, 0, 0));
		cr(c.startLayer());
		//c.printText("==Tilting/stepping up");
		


    }
    catch (JsonRpcException e)
    {
        cerr << e.what() << endl;
    }
	catch (const char * msg) 
	{
		cerr << msg << endl;
	}

	command_file.close();

    return 0;
}

