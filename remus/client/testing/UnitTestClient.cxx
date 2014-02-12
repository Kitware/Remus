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

  remus::client::ServerConnection test_full_sc("foo",82);
  remus::client::Client full_client(test_full_sc);

    REMUS_ASSERT( (full_client.connection().endpoint() ==
                        make_tcp_socket("foo",82).endpoint()) );
  REMUS_ASSERT( (full_client.connection().endpoint() != default_endpoint) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (full_client.connection().isLocalEndpoint()==false) );

}

} //namespace


int UnitTestClient(int, char *[])
{
  verify_server_connection();
  return 0;
}
