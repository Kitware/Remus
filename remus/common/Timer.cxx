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

#include <remus/common/Timer.h>

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/circular_buffer.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <numeric>

namespace remus{
namespace common{

//------------------------------------------------------------------------------
class Timer::TimeTracker
{
  typedef boost::posix_time::time_duration time_duration;
  typedef boost::posix_time::ptime ptime;
  typedef boost::posix_time::milliseconds milliseconds;

  ptime LastTime; //the exact time we last polled

public:
  TimeTracker():
    LastTime( boost::posix_time::microsec_clock::local_time() )
  {

  }

  //----------------------------------------------------------------------------
  void reset()
  {
    this->LastTime = boost::posix_time::microsec_clock::local_time();
  }

  //----------------------------------------------------------------------------
  boost::int64_t elapsed() const
  {
    const ptime currentTime = boost::posix_time::microsec_clock::local_time();
    const time_duration dur = currentTime - this->LastTime;
    return dur.total_milliseconds();
  }
};

//------------------------------------------------------------------------------
Timer::Timer():
  Tracker( new  Timer::TimeTracker() )
{
}

//------------------------------------------------------------------------------
Timer::~Timer()
{
}

//------------------------------------------------------------------------------
void Timer::reset()
{
  this->Tracker->reset();
}

//------------------------------------------------------------------------------
boost::int64_t Timer::elapsed() const
{
  return this->Tracker->elapsed();
}

}
}
