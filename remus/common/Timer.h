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
//=============================================================================

#ifndef remus_common_Timer_h
#define remus_common_Timer_h

#include <remus/common/CommonExports.h>
#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif

namespace remus{
namespace common{

// A class that can be used to time operations in remus.
// The system is built around boost::posix_time::microsec_clock::local_time
// so it should have micro-second resolution, but we report all
// elapsed times in milliseconds. To keep a consistent time scale with
// ZMQ, remus::common::PollingMonitor, remus::server::PollingRates, etc
class REMUSCOMMON_EXPORT Timer
{
public:
  // When a timer is constructed, the current time is marked so that elapsed()
  // returns the number of milliseconds elapsed since the construction.
  Timer();

  //explicit destructor required to properly determine the sizeof of TimeTracker
  ~Timer();

  // Resets the timer. All further calls to elapsed() will report the
  // number of milliseconds elapsed since the call to this.
  void reset();

  // Returns the elapsed time in milliseconds between the construction of this
  // class or the last call to reset and the time this function is called.
  boost::int64_t elapsed() const;

private:
  //explicitly state the timer doesn't support copy or move semantics
  Timer(const Timer &);
  void operator=(const Timer &);

  class TimeTracker;
  boost::scoped_ptr<TimeTracker> Tracker;
};

}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif

#endif
