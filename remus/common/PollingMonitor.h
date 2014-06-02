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

#ifndef remus_common_PollingMonitor_h
#define remus_common_PollingMonitor_h

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wcast-align"
#endif
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <remus/common/CommonExports.h>

namespace boost { namespace posix_time {  class ptime;  } }

namespace remus{
namespace common{

// Provides a dynamic heartbeat / polling monitor that adjusts
// the frequency of the polling rate based on the amount of traffic
// it receives. If the amount of traffic starts to decrease we increase
// the polling timeout.
class REMUSCOMMON_EXPORT PollingMonitor
{
public:
  //Create a PollingMonitor with the default min and max time out values of
  //5 seconds and 60 seconds
  PollingMonitor();

  //Create a PollingMonitor with user defined min and max time out values
  //if the max time
  PollingMonitor( boost::uint32_t MinTimeOutInSeconds,
                  boost::uint32_t MaxTimeOutInSeconds);

  PollingMonitor( const PollingMonitor& other );

  PollingMonitor& operator= (PollingMonitor other);

  ~PollingMonitor();

  //return the min timeout value as seconds
  boost::uint32_t minTimeOut() const;

  //return the max timeout value as seconds
  boost::uint32_t maxTimeOut() const;

  //mark that we just polled
  void pollOccurred( );

  //returns the amount of time from now to the last time we polled
  //the time is in seconds
  boost::int64_t durationFromLastPollMilliseconds() const;

  //returns the amount of time from now to the last time we polled
  //the time is in seconds
  boost::int64_t durationFromLastPoll() const;

  //returns the amount of time from now to the last time we polled
  //the time is in seconds
  boost::int64_t  durationOfTheLastPollMilliseconds() const;

  //returns the duration of time that between the last two times we polled
  //the time is in seconds
  boost::int64_t durationOfTheLastPoll() const;

  //retrieve the current poll rate in seconds
  boost::int64_t current() const;

  //retrieve the average poll rate in seconds
  boost::int64_t average() const;

  //returns true if the polling monitor has logged an abnormal
  //time being inserted. Over time this abnormal event will
  //be flushed from the system
  bool hasAbnormalEvent() const;

protected:
  //protected method that is mainly used for derived classes and testers
  //to verify the polling algorithm without having to actually wait in
  //real-time
  //All times must be forward in time or you will get undefined behavior
  virtual void pollOccurredAt( boost::posix_time::ptime* t );

private:
  class PollingTracker;
  boost::shared_ptr<PollingTracker> Tracker;
};

}
}

#endif
