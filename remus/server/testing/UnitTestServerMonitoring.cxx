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

#include <remus/server/Server.h>
#include <remus/server/ServerPorts.h>
#include <remus/testing/Testing.h>
#include <remus/common/SleepFor.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace {

//------------------------------------------------------------------------------
remus::server::ServerPorts make_inproc_ports()
{
  //generate random names for the channels
  std::string client_channel = remus::testing::UniqueString();
  std::string status_channel = remus::testing::UniqueString();
  std::string worker_channel = remus::testing::UniqueString();

  zmq::socketInfo<zmq::proto::inproc> ci(client_channel);
  zmq::socketInfo<zmq::proto::inproc> si(status_channel);
  zmq::socketInfo<zmq::proto::inproc> wi(worker_channel);
  return remus::server::ServerPorts(ci,si,wi);
}

//------------------------------------------------------------------------------
struct ThreadMonitor
{

  boost::scoped_ptr<boost::thread> Monitor;

  //----------------------------------------------------------------------------
  ThreadMonitor()
  {
  }

  //----------------------------------------------------------------------------
  ~ThreadMonitor()
  {
  this->Monitor->join();
  }

  //----------------------------------------------------------------------------
  void join_on_broker_started(remus::server::Server* server)
  {
  boost::scoped_ptr<boost::thread> sthread(
        new  boost::thread(&remus::server::Server::waitForBrokeringToStart, server) );
  this->Monitor.swap(sthread);
  }

  //----------------------------------------------------------------------------
  void join_on_broker_finished(remus::server::Server* server)
  {
  boost::scoped_ptr<boost::thread> sthread(
        new  boost::thread(&remus::server::Server::waitForBrokeringToFinish, server) );
  this->Monitor.swap(sthread);
  }

  //----------------------------------------------------------------------------
  bool good()
  {
    if(this->Monitor->joinable())
      {
      this->Monitor->join();
      return true;
      }
    return false;
  }
};

//------------------------------------------------------------------------------
void test_waitForBrokeringToStart()
{
  boost::shared_ptr<remus::Server> server( new remus::Server( make_inproc_ports() ) );

  ThreadMonitor tm;
  tm.join_on_broker_started(server.get());
  REMUS_ASSERT( (server->isBrokering() == false) );

  server->startBrokering();
  REMUS_ASSERT( (server->isBrokering() == true) );

  remus::common::SleepForMillisec(500);
  REMUS_ASSERT( (tm.good() == true) );

  REMUS_ASSERT( (server->isBrokering() == true) );
}

//------------------------------------------------------------------------------
void test_waitForBrokeringToFinish()
{
  boost::shared_ptr<remus::Server> server( new remus::Server( make_inproc_ports() ) );

  ThreadMonitor tm;
  tm.join_on_broker_finished(server.get());
  REMUS_ASSERT( (server->isBrokering() == false) );

  server->startBrokering();
  REMUS_ASSERT( (server->isBrokering() == true) );

  remus::common::SleepForMillisec(500);
  server->stopBrokering();

  REMUS_ASSERT( (tm.good() == true) );

  REMUS_ASSERT( (server->isBrokering() == false) );
}

} //namespace

int UnitTestServerMonitoring(int, char *[])
{
  test_waitForBrokeringToStart();
  test_waitForBrokeringToFinish();
  return 0;
}
