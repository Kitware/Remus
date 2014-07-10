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

#ifndef remus_server_detail_SocketMonitor_h
#define remus_server_detail_SocketMonitor_h

#include <boost/shared_ptr.hpp>

#include <remus/proto/Message.h>
#include <remus/proto/zmqSocketIdentity.h>

#include <remus/common/PollingMonitor.h>

namespace remus{
namespace server{
namespace detail{

// Provides monitoring that adjusts to the polling frequency of socket ids
class SocketMonitor
{
public:
  SocketMonitor( );
  SocketMonitor( remus::common::PollingMonitor pollingMonitor );
  SocketMonitor( const SocketMonitor& other );

  SocketMonitor& operator= (SocketMonitor other);

  ~SocketMonitor();

  //Returns the polling monitor, modifications of the returned object will
  //modify the socket monitor instance.
  remus::common::PollingMonitor pollingMonitor() const;

  //refresh a socket stating it is alive. Uses the pollingMontior
  //to determine the expect time of the next heartbeat from the socket.
  void refresh( const zmq::SocketIdentity& socket);

  //update a sockets heartbeat duration, marks the socket as alive.
  //Compares the heart beat interval in the message and the pollingMontior
  //to determine the expect time of the next heartbeat from the socket
  void heartbeat( const zmq::SocketIdentity& socket,
                  const remus::proto::Message& msg );

  //returns the interval in milliseconds between heartbeats for a socket.
  //This should always be a positive value.
  boost::int64_t heartbeatInterval( const zmq::SocketIdentity& socket) const;

  //we have been told by the socket it is shutting down, so we mark
  //the socket as fully dead.
  void markAsDead( const zmq::SocketIdentity& socket );

  //returns true if a socket is fully dead, and not mostly-dead
  bool isDead( const zmq::SocketIdentity& socket ) const;

  //returns true if a socket is not fully dead, but mostly-dead
  //this occurs when a socket has missed a heartbeat, but we some contextual
  //info from the polling monitor that some abnormal behavior has happened,
  //and we should expect sockets to come back.
  bool isUnresponsive( const zmq::SocketIdentity& socket ) const;

private:
  class WorkerTracker;
  boost::shared_ptr<WorkerTracker> Tracker;
};

}
}
}

#endif
