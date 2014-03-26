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

include_directories(${CMAKE_SOURCE_DIR})
set(twostep_SRC
	TwoStep.h
	TwoStep.cpp
	lasershark_hostapp/ls_ub_twostep_lib.h
	lasershark_hostapp/ls_ub_twostep_lib.c
	lasershark_hostapp/twostep_host_lib.h
	lasershark_hostapp/twostep_host_lib.c
	lasershark_hostapp/twostep_common_lib.h
	lasershark_hostapp/twostep_common_lib.c
	lasershark_hostapp/lasershark_uart_bridge_lib.h
	lasershark_hostapp/lasershark_uart_bridge_lib.c

)
add_library(twostep ${twostep_SRC})

