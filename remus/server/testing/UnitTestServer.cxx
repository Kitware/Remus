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
#include <remus/server/WorkerFactory.h>
#include <remus/testing/Testing.h>

#include <iostream>

namespace {

//presumes a != b
void test_server_ports(const remus::server::Server& a,
                       const remus::server::Server& b)
{
  const remus::server::ServerPorts& sa = a.ServerPortInfo();
  const remus::server::ServerPorts& sb = b.ServerPortInfo();

  //the server ports can't match on two servers since
  //when you create a server it binds the ports at that time
  //the host though will be the same
  REMUS_ASSERT( (sa.client().host() == sb.client().host()) );
  REMUS_ASSERT( (sa.client().port() != sb.client().port()) );
  REMUS_ASSERT( (sa.client().endpoint() != sb.client().endpoint()) );

  REMUS_ASSERT( (sa.worker().host() == sb.worker().host()) );
  REMUS_ASSERT( (sa.worker().port() != sb.worker().port()) );
  REMUS_ASSERT( (sa.worker().endpoint() != sb.worker().endpoint()) );
}

void test_server_constructors()
{
  //verify that all the constructors exist, and behave in the
  //expected manner. Note the class doesn't support copy
  //or move semantics currently
  remus::server::Server server_def;

  boost::shared_ptr<remus::server::WorkerFactory> factory(
                new remus::server::WorkerFactory());
  remus::server::Server server_fact( factory );

  remus::server::ServerPorts ports;
  remus::server::Server server_port( ports );

  remus::server::Server server_pf(ports,factory);

  //verify that each server has bound to a unique port, each
  //sharing the same hostname
  test_server_ports(server_def, server_fact);
  test_server_ports(server_def, server_port);
  test_server_ports(server_def, server_pf);
}

void test_server_sig_catching()
{
  void (*prev_sig_func)(int);
  {
  //construct a server that will not swallow signals since
  //it hasn't started brokering
  remus::server::Server server;

  //raise all signals, and verify that the default handler is catching
  //them
  prev_sig_func = signal( SIGABRT, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGFPE,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGILL,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGINT,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGSEGV, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGTERM, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  //now start the server for brokering and make sure it has
  //hooked onto all the signals
  server.startBrokering();

  prev_sig_func = signal( SIGABRT, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func != SIG_DFL) );

  prev_sig_func = signal( SIGFPE,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func != SIG_DFL) );

  prev_sig_func = signal( SIGILL,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func != SIG_DFL) );

  prev_sig_func = signal( SIGINT,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func != SIG_DFL) );

  prev_sig_func = signal( SIGSEGV, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func != SIG_DFL) );

  prev_sig_func = signal( SIGTERM, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func != SIG_DFL) );

  server.stopBrokering();
  //now the signals should be back to normal

  prev_sig_func = signal( SIGABRT, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGFPE,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGILL,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGINT,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGSEGV, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGTERM, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );


  server.startBrokering();
  }

  //now on destruction without calling stop brokering we should see
  //signal handling going back to defaults
  prev_sig_func = signal( SIGABRT, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGFPE,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGILL,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGINT,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGSEGV, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGTERM, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  //now create a server without signal catching

}

void test_server_brokering()
{
  //the all the different permutation of brokering and the destructor
  {
    remus::Server s;
   //make sure we destruct without brokering
  }

  {
    remus::Server s;
    s.startBrokering();
    //make sure we start brokering and can still destruct
  }

  {
    remus::Server s;
    s.startBrokering();
    s.startBrokering();
    s.startBrokering();
    s.startBrokering();
    s.startBrokering();
    s.startBrokering();
    //make sure multiple calls to start brokering is handled
  }

  {
    remus::Server s;
    s.startBrokering();
    s.stopBrokering();
    //make sure we start brokering and can still destruct
  }

  {
    remus::Server s;
    s.stopBrokering();
    //make sure we stop brokering works without starting
  }

  {
    remus::Server s;
    s.stopBrokering();
    s.startBrokering();
    s.stopBrokering();
    s.startBrokering();
    s.stopBrokering();
    s.stopBrokering();
    s.stopBrokering();
    s.startBrokering();
    //make sure we can start stop the brokering and destruct in the start
    //sate
  }
  {
    remus::Server s;
    s.stopBrokering();
    s.startBrokering();
    s.stopBrokering();
    s.startBrokering();
    s.startBrokering();
    s.startBrokering();
    s.stopBrokering();
    s.stopBrokering();
    s.stopBrokering();
    s.startBrokering();
    s.stopBrokering();
    //make sure we can start stop the brokering and destruct in the start
    //sate
  }

}

} //namespace

int UnitTestServer(int, char *[])
{
  //Test server construction
  test_server_constructors();

  //Test server signal catching
  test_server_sig_catching();

  //Test server brokering
  test_server_brokering();
  return 0;
}
