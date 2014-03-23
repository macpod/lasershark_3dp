# 
# This file is part of the LaserShark 3d Printer host application.
#
#  Adapted from cmake-modules Google Code project
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  (Changes for libusb) Copyright (c) 2008 Kyle Machulis <kyle@nonpolynomial.com>
# 
#  (Changes for json-rpc-cpp) Copyright (C) 2014 Jeffrey Nelson <nelsonjm@macpod.net>
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

# - Try to find jsonrpc
# Once done this will define
#
#  JSON_RPC_CPP_FOUND - system has libjsonrpc
#  JSON_RPC_CPP_INCLUDE_DIRS - the libjsonrpc include directory
#  JSON_RPC_CPP_LIBRARIES - Link these to use libjsonrpc
#
#  Adapted from cmake-modules Google Code project
#


if (JSON_RPC_CPP_LIBRARIES AND JSON_RPC_CPP_INCLUDE_DIRS)
  # in cache already
  set(LIBUSB_FOUND TRUE)
else (JSON_RPC_CPP_LIBRARIES AND JSON_RPC_CPP_INCLUDE_DIRS)
  find_path(JSON_RPC_CPP_INCLUDE_DIR
    NAMES
	jsonrpc.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
	PATH_SUFFIXES
	  jsonrpc
  )

  find_library(JSON_RPC_CPP_LIBRARY
    NAMES
      jsonrpc
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(JSON_RPC_CPP_INCLUDE_DIRS
    ${JSON_RPC_CPP_INCLUDE_DIR}
  )
  set(JSON_RPC_CPP_LIBRARIES
    ${JSON_RPC_CPP_LIBRARY}
)

  if (JSON_RPC_CPP_INCLUDE_DIRS AND JSON_RPC_CPP_LIBRARIES)
     set(JSON_RPC_CPP_FOUND TRUE)
  endif (JSON_RPC_CPP_INCLUDE_DIRS AND JSON_RPC_CPP_LIBRARIES)

  if (JSON_RPC_CPP_FOUND)
    if (NOT JSON_RPC_CPP_FIND_QUIETLY)
      message(STATUS "Found jsonrpc:")
	  message(STATUS " - Includes: ${JSON_RPC_CPP_INCLUDE_DIRS}")
	  message(STATUS " - Libraries: ${JSON_RPC_CPP_LIBRARIES}")
    endif (NOT JSON_RPC_CPP_FIND_QUIETLY)
  else (JSON_RPC_CPP_FOUND)
    if (JSON_RPC_CPP_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libusb")
    endif (JSON_RPC_CPP_FIND_REQUIRED)
  endif (JSON_RPC_CPP_FOUND)

  # show the JSON_RPC_CPP_INCLUDE_DIRS and JSON_RPC_CPP_LIBRARIES variables only in the advanced view
  mark_as_advanced(JSON_RPC_CPP_INCLUDE_DIRS JSON_RPC_CPP_LIBRARIES)

endif (JSON_RPC_CPP_LIBRARIES AND JSON_RPC_CPP_INCLUDE_DIRS)
