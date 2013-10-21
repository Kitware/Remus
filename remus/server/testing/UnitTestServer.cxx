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

  remus::server::WorkerFactory factory;
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
  //construct a server that will not swallow signals since
  //it hasn't started brokering
  remus::server::Server server;

  //raise all signals, and verify that the default handler is catching
  //them
  void (*prev_sig_func)(int);
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


  //ToDo: Now enable the brokering of the server and verify
  //that it is swallowing all signals

}

void test_server_brokering()
{
  //Todo: Nothing can happen intill the server is threaded.
}

} //namespace

int UnitTestServer(int, char *[])
{
  //Todo: Before we can have a real server test we need a way
  //to stop the server from brokering.

  //Test server construction
  test_server_constructors();


  //Test server signal catching
  test_server_sig_catching();


  //Test server brokering
  test_server_brokering();
  return 0;
}
