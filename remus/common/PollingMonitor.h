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
// PollingMonitor stores all objects behind a shared pointer, so make sure
// you don't need to create a new PollingMonitor before you modify the class
class REMUSCOMMON_EXPORT PollingMonitor
{
public:
  //Create a PollingMonitor with the default min and max time out values of
  //5 seconds and 60 seconds
  PollingMonitor();

  //Create a PollingMonitor with user defined min and max time out values.
  //Negative time out values are ignored and replaced with zero values. If
  //the Max and Min values are inverted we will swap the values.
  //
  //If you set Min and Max to both be very small values e.g. 0,2 expect
  //the server / worker that is consuming this class to be use a high level
  //of cpu.
  PollingMonitor( boost::int64_t MinTimeOutInMilliSeconds,
                  boost::int64_t MaxTimeOutInMilliSeconds);

  PollingMonitor( const PollingMonitor& other );

  PollingMonitor& operator= (PollingMonitor other);

  ~PollingMonitor();

  //Update a PollingMonitor with user defined min and max time out values.
  //Negative time out values are ignored and replaced with zero values. If
  //the Max and Min values are inverted we will swap the values.
  //
  //If you set Min and Max to both be very small values e.g. 0,2 expect
  //the server / worker that is consuming this class to be use a high level
  //of cpu.
  //
  //If the current values is currently outside the (min,max) range it
  //will be clamped to fall with the range.
  //
  void changeTimeOutRates(boost::int64_t minRate, boost::int64_t maxRate);

  //return the min timeout value as milliseconds
  boost::int64_t minTimeOut() const;

  //return the max timeout value as milliseconds
  boost::int64_t maxTimeOut() const;

  //mark that we just polled
  void pollOccurred( );

  //retrieve the current poll rate in milliseconds
  boost::int64_t current() const;

  //retrieve the average poll rate in milliseconds
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
