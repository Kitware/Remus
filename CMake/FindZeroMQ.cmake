#   Copyright 2011 SURFnet BV
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# - Try to find libzmq
# Once done, this will define
#
#  ZeroMQ_FOUND - system has libzmq
#  ZeroMQ_INCLUDE_DIRS - the libzmq include directories
#  ZeroMQ_LIBRARIES - link these to use libzmq

include(LibFindMacros)

IF (UNIX)
	# Use pkg-config to get hints about paths
	libfind_pkg_check_modules(ZeroMQ_PKGCONF libzmq)

	# Include dir
	find_path(ZeroMQ_INCLUDE_DIR
	  NAMES zmq.hpp
	  PATHS ${ZEROMQ_ROOT}/include ${ZeroMQ_PKGCONF_INCLUDE_DIRS}
	)

	# Finally the library itself
	find_library(ZeroMQ_LIBRARY
	  NAMES zmq
	  PATHS ${ZEROMQ_ROOT}/lib ${ZeroMQ_PKGCONF_LIBRARY_DIRS}
	)
ELSEIF (WIN32)
	find_path(ZeroMQ_INCLUDE_DIR
	  NAMES zmq.hpp
	  PATHS ${ZEROMQ_ROOT}/include ${CMAKE_INCLUDE_PATH}
	)
	# Finally the library itself
	find_library(ZeroMQ_LIBRARY
	  NAMES libzmq
	  PATHS ${ZEROMQ_ROOT}/lib ${CMAKE_LIB_PATH}
	)
ENDIF()

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(ZeroMQ_PROCESS_INCLUDES ZeroMQ_INCLUDE_DIR ZeroMQ_INCLUDE_DIRS)
set(ZeroMQ_PROCESS_LIBS ZeroMQ_LIBRARY ZeroMQ_LIBRARIES)
libfind_process(ZeroMQ)
