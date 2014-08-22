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

#include <remus/server/detail/SocketMonitor.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <algorithm>

namespace remus{
namespace server{
namespace detail{

//------------------------------------------------------------------------------
class SocketMonitor::WorkerTracker
{
  typedef boost::posix_time::ptime ptime;

  struct BeatInfo
    {
    boost::int64_t Duration;
    ptime LastOccurrence;
    };

public:
  remus::common::PollingMonitor PollMonitor;

  std::map< zmq::SocketIdentity, BeatInfo > HeartBeats;

  typedef std::pair< zmq::SocketIdentity, BeatInfo > InsertType;
  typedef std::map< zmq::SocketIdentity, BeatInfo >::iterator IteratorType;

  WorkerTracker( remus::common::PollingMonitor p):
    PollMonitor(p)
  {}

  //----------------------------------------------------------------------------
  remus::common::PollingMonitor monitor() const { return PollMonitor; }


  //----------------------------------------------------------------------------
  bool exists( const zmq::SocketIdentity& socket ) const
    { return this->HeartBeats.count(socket) > 0; }

  //----------------------------------------------------------------------------
  void refresh(const zmq::SocketIdentity& socket)
  {
    //insert a new item if it doesn't exist, otherwise get the beatInfo already
    //in the map
    InsertType key_value(socket,BeatInfo());
    IteratorType iter = (this->HeartBeats.insert(key_value)).first;
    BeatInfo& beat = iter->second;

    beat.LastOccurrence = boost::posix_time::microsec_clock::local_time();

    //look at the duration we have are currently polling at, and
    //the current duration that we last polled the worker at, take the slower
    //of the two. When the server has a really low heartbeat rate and the
    //and worker has a slow heartbeat we don't want to eliminate living workers
    const boost::int64_t polldur = (PollMonitor.hasAbnormalEvent() ? PollMonitor.maxTimeOut() : PollMonitor.current());
    beat.Duration = std::max(beat.Duration,polldur);
  }

  //----------------------------------------------------------------------------
  void heartbeat( const zmq::SocketIdentity& socket, boost::int64_t dur )
  {
    //insert a new item if it doesn't exist, otherwise get the beatInfo already
    //in the map
    InsertType key_value(socket,BeatInfo());
    IteratorType iter = (this->HeartBeats.insert(key_value)).first;
    BeatInfo& beat = iter->second;

    beat.LastOccurrence = boost::posix_time::microsec_clock::local_time();

    //Now we choose the greatest value between the poller and the sent in duration
    //from the socket
    const boost::int64_t polldur = (PollMonitor.hasAbnormalEvent() ? PollMonitor.maxTimeOut() : PollMonitor.current());
    beat.Duration = std::max(dur,polldur);
  }

  //----------------------------------------------------------------------------
  boost::int64_t heartbeatInterval(const zmq::SocketIdentity& socket)
  {
    //insert a new item if it doesn't exist, otherwise get the beatInfo already
    //in the map
    if(exists(socket))
      {
      InsertType key_value(socket,BeatInfo());
      IteratorType iter = (this->HeartBeats.insert(key_value)).first;
      BeatInfo& beat = iter->second;
      return beat.Duration;
      }
    return boost::int64_t(0);
  }

  //----------------------------------------------------------------------------
  void markAsDead( const zmq::SocketIdentity& socket )
  {
    this->HeartBeats.erase(socket);
  }

  //----------------------------------------------------------------------------
  bool isMostlyDead( const zmq::SocketIdentity& socket ) const
  {
    if(this->exists(socket))
      {
      //polling has been abnormal give it a pass
      if(PollMonitor.hasAbnormalEvent())
        {
        return false;
        }

      const BeatInfo& beat = this->HeartBeats.find(socket)->second;
      const ptime current = boost::posix_time::microsec_clock::local_time();
      const ptime expectedHB = beat.LastOccurrence +
                               boost::posix_time::milliseconds(beat.Duration*2);
      return current > expectedHB;
      }

    //the socket isn't contained here, this socket is dead dead
    return true;
  }
};

//------------------------------------------------------------------------------
SocketMonitor::SocketMonitor():
  Tracker(new WorkerTracker( (remus::common::PollingMonitor(25,60000)) ))
{

}

//------------------------------------------------------------------------------
SocketMonitor::SocketMonitor( remus::common::PollingMonitor pMonitor ):
  Tracker(new WorkerTracker( pMonitor ))
{

}

//------------------------------------------------------------------------------
SocketMonitor::SocketMonitor( const SocketMonitor& other ):
  Tracker(other.Tracker)
{

}

//------------------------------------------------------------------------------
SocketMonitor& SocketMonitor::operator= (SocketMonitor other)
{
  this->Tracker.swap(other.Tracker);
  return *this;
}


//------------------------------------------------------------------------------
SocketMonitor::~SocketMonitor()
{

}

//------------------------------------------------------------------------------
remus::common::PollingMonitor SocketMonitor::pollingMonitor() const
{
  return this->Tracker->monitor();
}

//------------------------------------------------------------------------------
void SocketMonitor::refresh( const zmq::SocketIdentity& socket )
{
  this->Tracker->refresh(socket);
}

//------------------------------------------------------------------------------
void SocketMonitor::heartbeat( const zmq::SocketIdentity& socket,
                               const remus::proto::Message& msg )
{
  //convert the string back into an int64_t which represents seconds to
  //the next heartbeat expected from this socket
  std::string messagePayload(msg.data(),msg.dataSize());
  boost::int64_t dur_in_milli = boost::lexical_cast<boost::int64_t>(
                                                              messagePayload);
  this->Tracker->heartbeat(socket, dur_in_milli);
}

//------------------------------------------------------------------------------
boost::int64_t SocketMonitor::heartbeatInterval(
                                    const zmq::SocketIdentity& socket) const
{
  return this->Tracker->heartbeatInterval(socket);
}

//------------------------------------------------------------------------------
void SocketMonitor::markAsDead( const zmq::SocketIdentity& socket )
{
  //we need to explicitly mark a socket as dead
  return this->Tracker->markAsDead(socket);
}

//------------------------------------------------------------------------------
bool SocketMonitor::isDead( const zmq::SocketIdentity& socket ) const
{
  return !this->Tracker->exists(socket);
}

//------------------------------------------------------------------------------
bool SocketMonitor::isUnresponsive( const zmq::SocketIdentity& socket ) const
{
  return this->Tracker->isMostlyDead(socket);
}

}
}
}
