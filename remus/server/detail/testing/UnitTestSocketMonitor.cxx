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

#include <remus/common/SleepFor.h>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/testing/Testing.h>

#include <boost/lexical_cast.hpp>

namespace
{
typedef remus::server::detail::SocketMonitor SocketMonitor;

//makes a random socket identity
zmq::SocketIdentity make_socketId()
{
  boost::uuids::uuid new_uid = remus::testing::UUIDGenerator();
  const std::string str_id = boost::lexical_cast<std::string>(new_uid);
  return zmq::SocketIdentity(str_id.c_str(),str_id.size());
}

//make a heartbeat message, duration in milliseconds
remus::proto::Message make_heartbeat( boost::int64_t dur_in_milli )
{
  return remus::proto::Message(remus::common::MeshIOType(),
                               remus::HEARTBEAT,
                               boost::lexical_cast<std::string>(dur_in_milli));
}


void verify_constructors()
{
  zmq::SocketIdentity sid = make_socketId();

  //create a socket monitor, add an Id to it and use that id to verify
  //that socket monitors shared pointer is working properly
  SocketMonitor monitor;
  monitor.refresh(sid); //add sid to the monitor


  SocketMonitor monitor2( monitor ); //share underlying state
  REMUS_ASSERT( (monitor2.isDead(sid) == monitor.isDead(sid)) );


  //not the same as monitor
  SocketMonitor monitor3;
  REMUS_ASSERT( (monitor3.isDead(sid) != monitor.isDead(sid)) );

  //now should be after the assignment operator
  {
  SocketMonitor tmp( monitor );
  monitor3 = tmp;
  }

  REMUS_ASSERT( (monitor3.isDead(sid) == monitor.isDead(sid)) );
}

void verify_bad_id()
{
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  REMUS_ASSERT( (monitor.isDead(sid) == true) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == true) );
}

void verify_existence()
{
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.refresh(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );

  zmq::SocketIdentity sid2 = make_socketId();
  monitor.heartbeat( sid2, make_heartbeat(250) );
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
}

void verify_markAsDead()
{
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.refresh(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  monitor.markAsDead(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == true) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == true) );
  }

  //do the same with heartbeating instead of refresh
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.heartbeat( sid, make_heartbeat(250) );
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  monitor.markAsDead(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == true) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == true) );
  }

}

void verify_resurrection()
{
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.refresh(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  monitor.markAsDead(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == true) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == true) );
  monitor.refresh(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  }

  //do the same with heartbeating instead of refresh
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.heartbeat( sid, make_heartbeat(250) );
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  monitor.markAsDead(sid);
  REMUS_ASSERT( (monitor.isDead(sid) == true) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == true) );
  monitor.heartbeat( sid, make_heartbeat(250) );
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  }
}

void verify_heartbeat_interval()
{
  //verify that the interval for unkown sockets is zero
  {
  SocketMonitor monitor;
  zmq::SocketIdentity sid = make_socketId();
  REMUS_ASSERT( (monitor.heartbeatInterval(sid) == 0) );
  }

  //check is to verify that negative heartbeats are properly ignored
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.heartbeat(sid, make_heartbeat(-5) ); //make a heartbeat of -5msec
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );

  REMUS_ASSERT( (monitor.heartbeatInterval(sid) > 0) );
  }

  //verify that a positive value heartbeat duration is stored correctly
  //in milliseconds
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;

  //change the timeout ranges to be smaller than the heartbeat value
  //we pass in so that we can very the heartbeat duration is the
  //interval value that we get back
  monitor.pollingMonitor().changeTimeOutRates(1,5);

  monitor.heartbeat(sid, make_heartbeat(10) ); //make a heartbeat of 10msec
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  REMUS_ASSERT( (monitor.heartbeatInterval(sid) == (10)) );


  //change the timeout ranges to be larger than the heartbeat value
  //we pass in so that we can very the heartbeat intervals is the min
  //time of the polling monitor
  monitor.pollingMonitor().changeTimeOutRates(100,500);

  monitor.heartbeat(sid, make_heartbeat(10) ); //make a heartbeat of 60sec
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  REMUS_ASSERT( (monitor.heartbeatInterval(sid) == (100)) );
  }
}

void verify_responiveness()
{
  //check is to verify that negative heartbeats are properly ignored
  {
  zmq::SocketIdentity sid = make_socketId();
  SocketMonitor monitor;
  monitor.heartbeat(sid, make_heartbeat(25) );
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );

  remus::common::SleepForMillisec(10);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  remus::common::SleepForMillisec(25);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );

  //after twice the interval sid's will become unresponsive
  remus::common::SleepForMillisec(25);
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == true) );

  //bring the socket back to being responsive
  monitor.heartbeat(sid, make_heartbeat(25) );
  REMUS_ASSERT( (monitor.isDead(sid) == false) );
  REMUS_ASSERT( (monitor.isUnresponsive(sid) == false) );
  }
}


}
int UnitTestSocketMonitor(int, char *[])
{
  //These tests don't really use the poller, but that is mainly verified by
  //the ActiveJobs and WorkerPool unit tests.
  verify_constructors();
  verify_bad_id();
  verify_existence();
  verify_markAsDead();
  verify_resurrection();
  verify_heartbeat_interval();
  verify_responiveness();

  return 0;
}
