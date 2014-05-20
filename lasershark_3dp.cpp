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
#include <string.h>

#include "LaserSharkJSONServer.h"
#include "LaserShark.h"

#include "TwoStepJSONServer.h"
#include "TwoStep.h"
#include "debug.h"


using namespace std;
using namespace jsonrpc;


bool do_exit = 0;


sigset_t mask, oldmask;

static void sig_hdlr(int signum)
{
    switch (signum) {
    case SIGINT:
        printf("\nGot request to quit\n");
        do_exit = 1;
        break;
    case SIGUSR1:
        printf("sigusr1 caught\n");
        do_exit = 1;
        break;
    default:
        printf("Received unexpected signal\n");
    }
}


void print_help(char* program)
{
    cout << program << "[--help|--lasershark_only]" << endl;
    cout << "\t--help - Prints this help text" << endl;
    cout << "\t--lasershark_only -- Initializes and uses LaserShark component only." << endl;
}


int main(int argc, char** argv)
{
    int rc;
    LaserShark ls;
    TwoStep ts;
    bool ls_only = false;
    struct sigaction sigact;

    if (argc > 2) {
        cerr << "Invalid args" << endl;
        print_help(argv[0]);
        return 1;
    }

    if (argc == 2) {
        if (0 == strcmp(argv[1], "--lasershark_only")) {
            ls_only = true;

        } else if (0 == strcmp(argv[1], "--help")) {
            print_help(argv[0]);
            return 0;
        } else {
            cerr << "Invalid args" << endl;
            print_help(argv[0]);
            return 1;
        }
    }

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

    try {
        if (!ls.connect()) {
            std::ostringstream oss;
            oss << "Could not connect to LaserShark.";
            throw std::runtime_error(oss.str());
        }
    } catch (runtime_error e) {
        cerr << e.what() << endl;
        return 1;
    }


    if (!ls_only) {
        try {
            if (!ts.connect()) {
                std::ostringstream oss;
                oss << "Could not connect to TwoStep via LaserShark.";
                throw std::runtime_error(oss.str());
            }
        } catch (runtime_error e) {
            cerr << e.what() << endl;

            cout << "LaserShark disconnecting" << endl;
            ls.disconnect();

            libusb_exit(NULL);
            return 1;
        }
    }

    try {
        LaserSharkJSONServer ls_serv;
        TwoStepJSONServer ts_serv;
        ls_serv.setLaserShark(&ls);
        if (!ls_only) {
            ts_serv.setTwoStep(&ts);
        }

        if (!ls_serv.StartListening()) {
            std::ostringstream oss;
            oss << "Error encountered initializing LaserShark JSON server.";
            throw std::runtime_error(oss.str());
        }

        if (!ls_only) {
            if (!ts_serv.StartListening()) {
                std::ostringstream oss;
                oss << "Error encountered initializing TwoStep JSON server.";
                throw std::runtime_error(oss.str());
            }
        }

        cout << "Servers started successfully. Type ctrl-c to quit." << endl;

        sigemptyset (&mask);
        sigaddset (&mask, SIGUSR1);

        sigprocmask (SIG_BLOCK, &mask, &oldmask);

        cout << "Entering loop" << endl;
        while (!do_exit) {
            sigsuspend (&oldmask);
            printf("Looping... (Must have recieved a signal, don't panic)\n");
        }
        sigprocmask (SIG_UNBLOCK, &mask, NULL);
        cout << "Exiting loop" << endl;

        ls_serv.StopListening();
        if (!ls_only) {
            ts_serv.StopListening();
        }
    } catch (jsonrpc::JsonRpcException& e) {
        cerr << e.what() << endl;
    } catch (runtime_error e) {
        cerr << e.what() << endl;
    }

    if (!ls_only) {
        cout << "TwoStep disconnecting" << endl;
        ts.disconnect();
    }

    cout << "LaserShark disconnecting" << endl;
    ls.disconnect();


    libusb_exit(NULL);
    return 0;
}

