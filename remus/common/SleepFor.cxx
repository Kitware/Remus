//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================

#include <remus/common/SleepFor.h>
#include <remus/common/CompilerInformation.h>

//First check if we have a C++11 compiler
#if defined(REMUS_HAVE_CXX_11)
  #include <chrono>
  #include <thread>
#elif defined(REMUS_MSVC)
  //For historical reasons, the Windows.h header defaults to including the
  //Winsock.h header file for Windows Sockets 1.1. The declarations in the
  //Winsock.h header file will conflict with the declarations in the Winsock2.h
  //header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN define
  //prevents the Winsock.h from being included by the Windows.h header.
  # ifndef WIN32_LEAN_AND_MEAN
  #   define WIN32_LEAN_AND_MEAN
  # endif
  #include <windows.h>
#else
  #include <time.h>
  #include <errno.h>
#endif

namespace remus {
namespace common {

void SleepForMillisec(int milliseconds)
{
#ifdef REMUS_HAVE_CXX_11

  //We have detected c++11 support, so use c++11 thread features
  const std::chrono::milliseconds dura( milliseconds );
  std::this_thread::sleep_for( dura );
#elif defined(_MSC_VER)

  //We are on older windows compilers, use Sleep
  Sleep(milliseconds);
#else

  //We are on a unix machine without c++11, so fallback to nanosleep,
  //which handles system interrupts
  struct timespec sleepTime;
  //extract the number of seconds to sleep for
  sleepTime.tv_sec = static_cast<time_t>(milliseconds/1000);

  //extract the remaining number of milliseconds;
  const long remaing_msec = static_cast<long>(milliseconds % 1000);

  //convert the milliseconds to nanoseconds
  const long MS_TO_NS_MULTIPLIER  = 1000000;
  sleepTime.tv_nsec = MS_TO_NS_MULTIPLIER * remaing_msec;

  //continue to sleep for the given duration of time. If we are interrupted
  //by an eintr signal resume sleeping
  while( (nanosleep( &sleepTime, &sleepTime ) != 0) && (errno == EINTR) );
#endif

}

}
}
