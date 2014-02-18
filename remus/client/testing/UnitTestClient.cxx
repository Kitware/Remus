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

#include <remus/client/ServerConnection.h>
#include <remus/client/Client.h>
#include <remus/testing/Testing.h>

#include <remus/proto/zmq.hpp>

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

void verify_server_connection()
{
  remus::client::ServerConnection default_sc;
  remus::client::Client default_client(default_sc);
  const remus::client::ServerConnection& sc = default_client.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  const std::string default_endpoint = sc.endpoint();

  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::SERVER_CLIENT_PORT);


  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );

  remus::client::ServerConnection ipc_conn =
        remus::client::make_ServerConnection("ipc://foo_ipc");
  remus::client::Client ipc_client(ipc_conn);

  REMUS_ASSERT( (ipc_client.connection().endpoint() ==
                 make_ipc_socket("foo_ipc").endpoint()) );


  remus::client::ServerConnection remote_tcp_conn =
        remus::client::make_ServerConnection("tcp://74.125.30.106:83");
  remus::client::Client remote_tcp_client(remote_tcp_conn);

  REMUS_ASSERT( (remote_tcp_client.connection().endpoint() ==
                 make_tcp_socket("74.125.30.106",83).endpoint()) );

  //We currently don't test with inproc since for inproc to work correctly
  //we need to share the same context between the client and calling code,
  //this would require a change on who holds the clients context

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (ipc_client.connection().isLocalEndpoint()==true) );
  REMUS_ASSERT( (remote_tcp_client.connection().isLocalEndpoint()==false) );

}

} //namespace


int UnitTestClient(int, char *[])
{
  verify_server_connection();
  return 0;
}
