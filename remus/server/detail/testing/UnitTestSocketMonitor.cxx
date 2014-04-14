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
#include <remus/proto/zmqSocketIdentity.h>

#include <remus/testing/Testing.h>

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
}

void verify_markAsDead()
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

void verify_resurrection()
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

}
int UnitTestSocketMonitor(int, char *[])
{
  //Doesn't test heartbeating, just refreshing, that is pretty complex
  //to test, and I currently don't have the time to write a standalone tests
  //for tat.

  //These tests don't really use the poller, but that is mainly verified by
  //the ActiveJobs and WorkerPool unit tests.
  verify_constructors();
  verify_bad_id();
  verify_existence();
  verify_markAsDead();
  verify_resurrection();


  return 0;
}
