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
#include <remus/worker/Worker.h>
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
  using namespace remus::meshtypes;
  const remus::common::MeshIOType mtype =
                          remus::common::make_MeshIOType(Model(),Model());

  remus::worker::ServerConnection default_sc;
  remus::worker::Worker default_worker(mtype,default_sc);
  const remus::worker::ServerConnection& sc = default_worker.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  const std::string default_endpoint = sc.endpoint();

  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::SERVER_WORKER_PORT);


  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );

  remus::worker::ServerConnection ipc_conn =
        remus::worker::make_ServerConnection("ipc://foo_ipc");
  remus::worker::Worker ipc_worker(mtype,ipc_conn);

  REMUS_ASSERT( (ipc_worker.connection().endpoint() ==
                 make_ipc_socket("foo_ipc").endpoint()) );


  remus::worker::ServerConnection remote_tcp_conn =
        remus::worker::make_ServerConnection("tcp://74.125.30.106:83");
  remus::worker::Worker remote_tcp_worker(mtype,remote_tcp_conn);

  REMUS_ASSERT( (remote_tcp_worker.connection().endpoint() ==
                 make_tcp_socket("74.125.30.106",83).endpoint()) );


  remus::worker::ServerConnection inproc_conn =
         remus::worker::make_ServerConnection("inproc://foo_inproc");

  //to properly connect to an inproc_socket you first have something
  //bind to the name, otherwise the connecting socket just hangs. Also
  //the same zmq context needs to be shared between the binding and connecting
  //socket.
  zmq::socket_t inproc_bound_socket( *(inproc_conn.context()), ZMQ_REP );
  inproc_bound_socket.bind(inproc_conn.endpoint().c_str());

  remus::worker::Worker inproc_worker(mtype,inproc_conn);

  REMUS_ASSERT( (inproc_worker.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );

  //share a context and connection info between two workers
  remus::worker::Worker inproc_worker2(mtype,inproc_worker.connection());
  REMUS_ASSERT( (inproc_worker2.connection().endpoint() == make_inproc_socket("foo_inproc").endpoint()) );
  REMUS_ASSERT( (inproc_worker.connection().context() ==
                 inproc_worker2.connection().context() ) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (ipc_worker.connection().isLocalEndpoint()==true) );
  REMUS_ASSERT( (inproc_worker.connection().isLocalEndpoint()==true) );
  REMUS_ASSERT( (remote_tcp_worker.connection().isLocalEndpoint()==false) );


}

} //namespace


int UnitTestWorker(int, char *[])
{
  verify_server_connection();
  return 0;
}
