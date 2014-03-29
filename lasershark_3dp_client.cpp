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

#include <unistd.h>

#include "base64/base64_cpp.h"
#include "lasersharkjsonclient.h"
#include "twostepjsonclient.h"
#include "twostep_common_lib.h"

using namespace jsonrpc;
using namespace std;


Json::Value cr(Json::Value value) throw (std::runtime_error)
{
    if (!value["success"].asBool()) {
        std::ostringstream oss;
        oss << value["message"];
        throw std::runtime_error(oss.str());
    }
    return value;
}


// consider max size restriction. Consider quick check to see if this file is valid.
char* read_and_base64_encode_image(const char* image_file)
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
    if (encoded == NULL	) {
        delete[] file_data;
        return NULL;
    }

    return encoded;
}


void sendAndWaitForLayer(LaserSharkJSONClient &lsc, string file_name, unsigned int sleep_delay) throw (std::runtime_error)
{
    Json::Value value;
    char* base64_img = read_and_base64_encode_image(file_name.c_str());
    if (!base64_img) {
        std::ostringstream oss;
        oss << "Could not read or encode image";
        throw std::runtime_error(oss.str());
    }

    cout << "Sending Layer" << endl;
    cr(lsc.sendLayer(base64_img, 0, 0));
    cout << "Starting Layer" << endl;
    cr(lsc.startLayer());
    int totalSamples = cr(lsc.getLayerTotalSamples())["value"].asInt();
    while(1) {
        sleep(sleep_delay);
        int layerSamplesLeft = cr(lsc.getLayerSamplesLeft())["value"].asInt();
        value = cr(lsc.getLayerDone());
        if (value["value"].asBool() == false) {
            cout << "Completed " << layerSamplesLeft << " of " << totalSamples << " layer samples." << endl;
        } else {
            cout << "Layer Completed" << endl;
            break;
        }
    }

}

void initializeSteppers(TwoStepJSONClient &tsc) throw (std::runtime_error)
{
    cout << "Initializing steppers" << endl;
    cr(tsc.stop(true, true));
    cr(tsc.setMicrosteps(TWOSTEP_STEPPER_1, TWOSTEP_MICROSTEP_BITFIELD_FULL_STEP));
    cr(tsc.setMicrosteps(TWOSTEP_STEPPER_2, TWOSTEP_MICROSTEP_BITFIELD_FULL_STEP));
    cr(tsc.setCurrent(TWOSTEP_STEPPER_1, 50/*TWOSTEP_MIN_CURRENT_VAL*/));
    cr(tsc.setCurrent(TWOSTEP_STEPPER_2, 50/*TWOSTEP_MIN_CURRENT_VAL*/));
    cr(tsc.set100uSDelay(TWOSTEP_STEPPER_1, TWOSTEP_STEP_100US_DELAY_5MS));
    cr(tsc.setEnable(TWOSTEP_STEPPER_1, true));
    cr(tsc.setEnable(TWOSTEP_STEPPER_2, true));
}


void waitForStepperToStop(TwoStepJSONClient &tsc, int stepperNum) throw (std::runtime_error)
{
    while (cr(tsc.getIsMoving(stepperNum))["value"].asBool()) {
        sleep(1);
    }
}


void performHomingSequence(TwoStepJSONClient &tsc) throw (std::runtime_error)
{
    cout << "Performing homing sequence" << endl;
    cout << "NOTICE: Code commented out ... please see source" << endl;
    // This is going to be machine dependent. The following is only an example.
    // Don't forget you can query twostep attributes so they can be restored after changing them!
    /*
    	// Set homing speed to be fast, specify stepper should move until a relay starts and then run!
        cr(tsc.set100uSDelay(TWOSTEP_STEPPER_1, TWOSTEP_STEP_100US_DELAY_5MS));
    	cr(tsc.setDir(TWOSTEP_STEPPER_1, TWOSTEP_STEPPER_DIR_HIGH));
    	cr(tsc.setStepUntilSwitch(TWOSTEP_STEPPER_1));
    	cr(tsc.start(true, false));
    	waitForStepperToStop(tsc, TWOSTEP_STEPPER_1);

    	// Back up some by some arbitrarily set number of steps.
    	cr(tsc.setDir(TWOSTEP_STEPPER_1, TWOSTEP_STEPPER_DIR_LOW));
    	cr(tsc.setSafeSteps(TWOSTEP_STEPPER_1, 50));
    	cr(tsc.start(true, false));
    	waitForStepperToStop(tsc, TWOSTEP_STEPPER_1);

    	// Set slow homing speed and slowly home.
        cr(tsc.set100uSDelay(TWOSTEP_STEPPER_1, TWOSTEP_STEP_100US_DELAY_5MS*5));
    	cr(tsc.setDir(TWOSTEP_STEPPER_1, TWOSTEP_STEPPER_DIR_HIGH));
    	cr(tsc.setStepUntilSwitch(TWOSTEP_STEPPER_1));
    	cr(tsc.start(true, false));
    	waitForStepperToStop(tsc, TWOSTEP_STEPPER_1);
    */

}


void performInterStepSequence(TwoStepJSONClient &tsc) throw (std::runtime_error)
{
    cout << "Running inter-layer sequence" << endl;
    // This is going to be machine dependent. The following is here to demonstrate
    // how two stepper motors can be configured differently yet still started at the
    // same time.

    // Configure Stepper 1 to do one thing...
    cr(tsc.setDir(TWOSTEP_STEPPER_1, TWOSTEP_STEPPER_DIR_LOW));
    cr(tsc.setSafeSteps(TWOSTEP_STEPPER_1, 500));

    // Configure Stepper 2 to do another thing...
    cr(tsc.setDir(TWOSTEP_STEPPER_2, TWOSTEP_STEPPER_DIR_HIGH));
    cr(tsc.setSafeSteps(TWOSTEP_STEPPER_2, 1000));
    cr(tsc.start(true, true));
    waitForStepperToStop(tsc, TWOSTEP_STEPPER_1);
    waitForStepperToStop(tsc, TWOSTEP_STEPPER_2);
}


void performPostFinalLayerSequence(TwoStepJSONClient &tsc) throw (std::runtime_error)
{
    cout << "Running post-final layer sequence" << endl;
    // This is going to be machine dependent. The following is here to demonstrate
    // how two stepper motors can be started at different times yet stopped at the same time

    // Configure Stepper 1 to do one thing...
    cr(tsc.setDir(TWOSTEP_STEPPER_1, TWOSTEP_STEPPER_DIR_LOW));
    cr(tsc.setStepUntilSwitch(TWOSTEP_STEPPER_1));
    cr(tsc.start(true, false));
    sleep(1);
    // Configure Stepper 2 to do another thing...
    cr(tsc.setDir(TWOSTEP_STEPPER_2, TWOSTEP_STEPPER_DIR_LOW));
    cr(tsc.setStepUntilSwitch(TWOSTEP_STEPPER_2));
    cr(tsc.start(false, true));
    sleep(2);

    cr(tsc.stop(true, true));
}

void performShutdownSequence(TwoStepJSONClient &tsc) throw (std::runtime_error)
{
    cout << "Shutting down steppers" << endl;
    cr(tsc.stop(true, true));
    cr(tsc.setEnable(TWOSTEP_STEPPER_1, false));
    cr(tsc.setEnable(TWOSTEP_STEPPER_2, false));
}


int main(int argc, char** argv)
{
    Json::Value value;

    if (argc != 2) {
        cout << " Must specify png file" << endl;
        return 1;
    }

    char *file_name = argv[1];

    LaserSharkJSONClient lsc(new HttpClient("http://localhost:8080"));
    TwoStepJSONClient tsc(new HttpClient("http://localhost:8081"));

    try {
        tsc.printText("Hello from client!");
        lsc.printText("Hello from client!");

        initializeSteppers(tsc);

        cout << "Setting lasershark sample rate" << endl;
        cr(lsc.setSampleRate(20000));

        performHomingSequence(tsc);

        sendAndWaitForLayer(lsc, file_name, 1);
        performInterStepSequence(tsc);
        cout << "Sleeping" << endl;
        sleep(5);
        performPostFinalLayerSequence(tsc);


        tsc.printText("Goodbye from client!");
        lsc.printText("Goodbye from client!");

    } catch (JsonRpcException e) {
        cerr << e.what() << endl;
    } catch (runtime_error e) {
        cerr << e.what() << endl;
    }

    try {
        performShutdownSequence(tsc);
    } catch (JsonRpcException e) {
        cerr << e.what() << endl;
    } catch (runtime_error e) {
        cerr << e.what() << endl;
    }

    return 0;
}

