# 
# This file is part of the LaserShark 3d Printer host application.
# 
# Copyright (C) 2014 Jeffrey Nelson <nelsonjm@macpod.net>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# 


# pass the following to cmake to include symbols for gdb.
#-DCMAKE_BUILD_TYPE=Debug
# pass one of the following to cmake to to enable/disable debug printing.
#-DDEBUG_PRINT=ON
#-DDEBUG_PRINT=OFF


project(lasershark_3d_printer)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Modules/")

find_package(json-rpc-cpp REQUIRED)
find_package(libusb-1.0 REQUIRED)


set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_C_FLAGS "-Wall")


set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")

option(DEBUG_PRINT "Print debug messages" OFF)

if (DEBUG_PRINT)
message("-- Debug printing enabled")
add_definitions(-DDEBUG_PRINT)
else (DEBUG_PRINT)
message("-- Debug printing disabled")
endif (DEBUG_PRINT)



add_subdirectory (base64)
add_subdirectory (lodepng)

include_directories(${LIBUSB_1_INCLUDE_DIRS})

add_subdirectory(twostep)
add_subdirectory (lasershark)

include_directories(${JSON_RPC_CPP_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/twostep)
include_directories(${CMAKE_SOURCE_DIR}/lasershark)


set(lasershark_3dp_SRC 
	lasershark_3dp.cpp
	LaserSharkJSONServer.cpp
	TwoStepJSONServer.cpp
	debug.h
)

add_executable(lasershark_3dp ${lasershark_3dp_SRC})
target_link_libraries (lasershark_3dp ${LIBUSB_1_LIBRARIES} ${JSON_RPC_CPP_LIBRARIES} base64 lodepng lasershark twostep)




set(lasershark_3dp_client_SRC
	lasershark_json_client.cpp
	lasersharkjsonclient.h
	debug.h
)

add_executable(lasershark_3dp_client ${lasershark_3dp_client_SRC})
target_link_libraries (lasershark_3dp_client ${JSON_RPC_CPP_LIBRARIES} base64)



add_custom_target (twostep_stubs
    COMMAND jsonrpcstub -s -c -o ./ twostep_spec.json TwoStepJSON
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
	VERBATIM
)

add_custom_target (lasershark_stubs
    COMMAND jsonrpcstub -s -c -o ./ lasershark_spec.json LaserSharkJSON
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
	VERBATIM
)


