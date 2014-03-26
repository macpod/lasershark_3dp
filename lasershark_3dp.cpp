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
#include <fstream>
#include <exception>
#include <unistd.h>
#include <signal.h>

#include "LaserSharkJSONServer.h"
#include "LaserShark.h"

#include "TwoStepJSONServer.h"
#include "TwoStep.h"
#include "debug.h"

/*
out base64 library doesn't seem to be able to decode newlines in blocks very well.
correct throws (shouldn't be strings)
see if we need to check for connected state for lasershark/twostep or if it will work fine as-is
for both twostep and lasershark, shouldn't you check the version before running? Also shouldn't you bail and disconnect if getting initial conditions fails?
	yes.

*/

using namespace std;
using namespace jsonrpc;


bool do_exit = 0;


sigset_t mask, oldmask;

static void sig_hdlr(int signum)
{
    switch (signum)
    {
    case SIGINT:
        printf("\nGot request to quit\n");
        do_exit = 1;
        break;
    case SIGUSR1:
        printf("sigusr1 caught\n");
        do_exit = 1;
        break;
    default:
        printf("what\n");
    }
}


int main(int argc, char** argv)
{
	int rc;
	LaserShark ls;
	TwoStep ts;
    struct sigaction sigact;


    rc = libusb_init(NULL);
	if (rc < 0) {
		cerr << "Error initializing libusb: " << libusb_error_name(rc) << endl;
		return 1;
	}


    sigact.sa_handler = sig_hdlr;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);

    try
    {
        if (!ls.connect()) {
			cerr << "Could not connect to lasershark" << endl;
			return 1;
		}
    }
    catch (runtime_error e)
    {
        cerr << e.what() << endl;
        return 1;
    }


    try
    {
        if (!ts.connect()) {
			cerr << "Could not connect to twostep via lasershark" << endl;
			return 1;
		}
    }
    catch (runtime_error e)
    {
        cerr << e.what() << endl;
    	try
    	{
    	    cout << "lasershark disconnecting...starting" << endl;
    	    ls.disconnect();
    	    cout << "lasershark disconnecting...end" << endl;

    	}
    	catch (runtime_error e)
    	{
    	    cerr << e.what() << endl;
 	   	}
		libusb_exit(NULL);		
        return 1;
    }



    try
    {
        LaserSharkJSONServer serv;
		serv.setLaserShark(&ls);
        if (serv.StartListening())
        {

            cout << "Server started successfully. Type ctrl-c to quit." << endl;

		    sigemptyset (&mask);
		    sigaddset (&mask, SIGUSR1);

    		sigprocmask (SIG_BLOCK, &mask, &oldmask);

			cout << "Entering loop" << endl;
			while (!do_exit) {
			    sigsuspend (&oldmask);
			    printf("Looping... (Must have recieved a signal, don't panic).\n");
			}
		    sigprocmask (SIG_UNBLOCK, &mask, NULL);
			cout << "Exiting loop" << endl;
			ls.stopAndClearLayer();

            serv.StopListening();
        }
        else
        {
            cout << "Error starting Server" << endl;
        }
    }
    catch (jsonrpc::JsonRpcException& e)
    {
        cerr << e.what() << endl;
    }
    catch (const char * msg)
    {
        cerr << msg << endl;
    }


    try
    {
        cout << "lasershark disconnecting...starting" << endl;
        ls.disconnect();
        cout << "lasershark disconnecting...end" << endl;
    }
    catch (runtime_error e)
    {
        cerr << e.what() << endl;
    }


    try
    {
        cout << "twostep disconnecting...starting" << endl;
        ts.disconnect();
        cout << "twostep disconnecting...end" << endl;
    }
    catch (runtime_error e)
    {
        cerr << e.what() << endl;
    }


	libusb_exit(NULL);
    return 0;
}

