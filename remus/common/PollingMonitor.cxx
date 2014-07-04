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

#include <remus/common/PollingMonitor.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/circular_buffer.hpp>

#include <algorithm>
#include <numeric>

namespace remus{
namespace common{

//------------------------------------------------------------------------------
class PollingMonitor::PollingTracker
{
  typedef boost::posix_time::time_duration time_duration;
  typedef boost::posix_time::ptime ptime;
  typedef boost::posix_time::milliseconds milliseconds;

  time_duration MinTimeOut; //min heartbeat interval
  time_duration MaxTimeOut; //min heartbeat interval

  time_duration AveragePollRate;  //current duration to poll for
  time_duration CurrentPollRate;  //current duration to poll for
  ptime LastPollTime; //the exact time we last polled

  //the frequency we have polled in the past
  boost::circular_buffer< time_duration > PollingFrequency;

public:
  PollingTracker( const boost::int64_t minTimeoutInMillis,
                  const boost::int64_t maxTimeoutInMillis,
                  const ptime& p ):
    MinTimeOut( milliseconds( std::min(minTimeoutInMillis,maxTimeoutInMillis)) ),
    MaxTimeOut( milliseconds( std::max(minTimeoutInMillis,maxTimeoutInMillis)) ),
    AveragePollRate(  milliseconds( std::min(minTimeoutInMillis,maxTimeoutInMillis)) ),
    CurrentPollRate(  milliseconds( std::min(minTimeoutInMillis,maxTimeoutInMillis)) ),
    LastPollTime(p),
    PollingFrequency(5)
  {
    this->PollingFrequency.push_back( this->MinTimeOut );
  }

  const time_duration& minTimeOut() const { return MinTimeOut; }
  const time_duration& maxTimeOut() const { return MaxTimeOut; }

  //----------------------------------------------------------------------------
  void pollOccurred()
  {
    const ptime currentTime = boost::posix_time::microsec_clock::local_time();
    this->pollOccurred(currentTime);
  }

  //----------------------------------------------------------------------------
  void pollOccurred( const ptime& time  )
  {
    const time_duration dur = time - this->LastPollTime;

    //take the time duration between the last poll time, and the current time.
    //that gets us the entry to add to the polling frequency
    this->PollingFrequency.push_back( dur );

    time_duration sum = std::accumulate(this->PollingFrequency.begin(),
                                        this->PollingFrequency.end(),
                                        time_duration() );
    time_duration avg = sum / this->PollingFrequency.size();

    //update the member vars
    this->LastPollTime = time;
    this->AveragePollRate = avg;

    if(avg < this->MinTimeOut )
      {
      //if we are less than the min return the min as the poll time,
      //as this means we have enough communication to make the poll timeout
      //less relevant
      this->CurrentPollRate = this->MinTimeOut;
      }
    else
      {
      boost::int64_t mil_secs = static_cast<boost::int64_t>(avg.total_milliseconds() * 0.25);
      this->CurrentPollRate = avg + milliseconds(mil_secs);
      if(this->CurrentPollRate > this->MaxTimeOut)
        {
        //we hit our cap for how slow we can heartbeat
        this->CurrentPollRate = this->MaxTimeOut;
        }
      }

  }

  //----------------------------------------------------------------------------
  time_duration durationFromLastPoll() const
  {
    const ptime currentTime = boost::posix_time::microsec_clock::local_time();
    return currentTime - this->LastPollTime;
  }

  //----------------------------------------------------------------------------
  const time_duration& durationOfTheLastPoll() const
  {
    return this->PollingFrequency.back();
  }

  //----------------------------------------------------------------------------
  //returns the current poll rate
  const time_duration& current() const
  {
    return this->CurrentPollRate;
  }

  //----------------------------------------------------------------------------
  //returns the average poll rate
  const time_duration& average() const
  {
    return this->AveragePollRate;
  }
};

//------------------------------------------------------------------------------
PollingMonitor::PollingMonitor():
  Tracker(new PollingMonitor::PollingTracker( (5*1000), (60*1000),
            boost::posix_time::microsec_clock::local_time() ) )
{
}

//------------------------------------------------------------------------------
PollingMonitor::PollingMonitor( boost::int64_t MinTimeOutInMilliSeconds,
                                boost::int64_t MaxTimeOutInMilliSeconds):
  Tracker(new PollingMonitor::PollingTracker(
            MinTimeOutInMilliSeconds,
            MaxTimeOutInMilliSeconds,
            boost::posix_time::microsec_clock::local_time() ) )
{

}

//------------------------------------------------------------------------------
PollingMonitor::PollingMonitor( const PollingMonitor& other ):
  Tracker(other.Tracker)
{
}

//------------------------------------------------------------------------------
PollingMonitor& PollingMonitor::operator= (PollingMonitor other)
{
  this->Tracker.swap(other.Tracker);
  return *this;
}

//------------------------------------------------------------------------------
PollingMonitor::~PollingMonitor()
{

}

//------------------------------------------------------------------------------
boost::int64_t PollingMonitor::minTimeOut() const
{
  return this->Tracker->minTimeOut().total_milliseconds();
}

//------------------------------------------------------------------------------
boost::int64_t PollingMonitor::maxTimeOut() const
{
  return this->Tracker->maxTimeOut().total_milliseconds();
}

//------------------------------------------------------------------------------
void PollingMonitor::pollOccurred()
{
  this->Tracker->pollOccurred();
}

//------------------------------------------------------------------------------
void PollingMonitor::pollOccurredAt( boost::posix_time::ptime* t )
{
  this->Tracker->pollOccurred( *t );
}

//------------------------------------------------------------------------------
boost::int64_t PollingMonitor::durationFromLastPoll() const
{
  return this->Tracker->durationFromLastPoll().total_milliseconds();
}

//------------------------------------------------------------------------------
boost::int64_t PollingMonitor::durationOfTheLastPoll() const
{
  return this->Tracker->durationOfTheLastPoll().total_milliseconds();
}

//------------------------------------------------------------------------------
boost::int64_t PollingMonitor::current() const
{
  return this->Tracker->current().total_milliseconds();
}

//------------------------------------------------------------------------------
boost::int64_t PollingMonitor::average() const
{
  return this->Tracker->average().total_milliseconds();
}

//------------------------------------------------------------------------------
bool PollingMonitor::hasAbnormalEvent() const
{
  return this->Tracker->average() > this->Tracker->current();
}


}
}
