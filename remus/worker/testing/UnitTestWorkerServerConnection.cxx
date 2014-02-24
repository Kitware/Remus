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

#include <remus/worker/ServerConnection.h>
#include <remus/testing/Testing.h>

#include <string>

namespace {

zmq::socketInfo<zmq::proto::tcp> make_tcp_socket(std::string host, int port)
{
  return zmq::socketInfo<zmq::proto::tcp>(host,port);
}

zmq::socketInfo<zmq::proto::ipc> make_ipc_socket(std::string host)
{
  return zmq::socketInfo<zmq::proto::ipc>(host);
}

zmq::socketInfo<zmq::proto::inproc> make_inproc_socket(std::string host)
{
  return zmq::socketInfo<zmq::proto::inproc>(host);
}


} //namespace


int UnitTestWorkerServerConnection(int, char *[])
{

  remus::worker::ServerConnection sc;
  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  const std::string default_endpoint = sc.endpoint();

  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::SERVER_WORKER_PORT);


  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );

  remus::worker::ServerConnection test_socket_sc(default_socket);
  remus::worker::ServerConnection test_socket_sc2(
                                                make_tcp_socket("127.0.0.1",1));

  REMUS_ASSERT( (test_socket_sc.endpoint() == default_endpoint) );
  REMUS_ASSERT( (test_socket_sc2.endpoint() != default_endpoint) );

  remus::worker::ServerConnection test_full_sc("74.125.30.106",82);
  remus::worker::ServerConnection test_full_sc2 =
              remus::worker::make_ServerConnection("tcp://74.125.30.106:82");

  REMUS_ASSERT( (test_full_sc.endpoint() ==
                        make_tcp_socket("74.125.30.106",82).endpoint()) );
  REMUS_ASSERT( (test_full_sc.endpoint() == test_full_sc2.endpoint()) );
  REMUS_ASSERT( (test_full_sc.endpoint() != default_endpoint) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (test_socket_sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (test_socket_sc2.isLocalEndpoint()==true) );
  REMUS_ASSERT( (test_full_sc.isLocalEndpoint()==false) );

  //test inproc
  remus::worker::ServerConnection sc_inproc( make_inproc_socket("worker") );
  REMUS_ASSERT( (sc_inproc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (sc_inproc.endpoint() == std::string("inproc://worker")) );

  remus::worker::ServerConnection sc_inproc2 =
                      remus::worker::make_ServerConnection("inproc://worker");
  REMUS_ASSERT( (sc_inproc2.isLocalEndpoint()==true) );
  REMUS_ASSERT( (sc_inproc2.endpoint() == std::string("inproc://worker")) );

  //test ipc
  remus::worker::ServerConnection sc_ipc( make_ipc_socket("task_pool") );
  REMUS_ASSERT( (sc_ipc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (sc_ipc.endpoint() == std::string("ipc://task_pool")) );

  remus::worker::ServerConnection sc_ipc2 =
                      remus::worker::make_ServerConnection("ipc://task_pool");
  REMUS_ASSERT( (sc_ipc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (sc_ipc.endpoint() == std::string("ipc://task_pool")) );


  return 0;
}
