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
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#include <remus/client/Client.h>
#include <remus/server/Server.h>
#include <remus/worker/Worker.h>

#include <remus/testing/integration/detail/Helpers.h>

namespace
{
  namespace detail
  {
  using namespace remus::testing::integration::detail;
  }

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  boost::shared_ptr<remus::Server> server( new remus::Server(ports) );
  server->startBrokering();
  return server;
}

//------------------------------------------------------------------------------
void swap_worker_connection( boost::shared_ptr<remus::Worker> worker )
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  //new connection
  remus::worker::ServerConnection conn;

  //new reqs
  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements requirements = make_JobRequirements(io_type, "SwapWorker", "");
  remus::Worker* swapWorker = new remus::Worker(requirements,conn);
  worker.reset(swapWorker);
}

//------------------------------------------------------------------------------
void swap_client_connection( boost::shared_ptr<remus::Client> client )
{
  //new connection
  remus::client::ServerConnection conn;

  //new reqs
  remus::Client* swapClient = new remus::Client(conn);
  client.reset(swapClient);
}
}


//Verify that we can share contexts between client, server and worker
int ShareContext(int argc, char* argv[])
{
  using namespace remus::meshtypes;

  (void) argc;
  (void) argv;

  //construct a simple worker and client
  remus::server::ServerPorts tcp_ports = remus::server::ServerPorts();
  boost::shared_ptr<remus::Server> server = make_Server( tcp_ports );
  tcp_ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> tcp_client = detail::make_Client( tcp_ports, true );

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  boost::shared_ptr<remus::Worker> tcp_worker = detail::make_Worker( tcp_ports, io_type, "SimpleWorker", true );
  boost::shared_ptr<remus::Worker> tcp_worker2 = detail::make_Worker( tcp_ports, io_type, "SimpleWorker", true );

  //verify we can create multiple client contexts and swap clients.
  //zmq context will seg-fault if this doesn't work.
  swap_client_connection( detail::make_Client( tcp_ports, true ) );
  swap_client_connection( tcp_client );

  //verify we can create multiple worker context and swap workers.
  //zmq context will seg-fault if this doesn't work.
  swap_worker_connection( detail::make_Worker( tcp_ports, io_type, "SimpleWorker", true ) );
  swap_worker_connection( tcp_worker2 );

  //generate random names for the channels
  boost::uuids::random_generator generator;
  std::string client_channel = boost::uuids::to_string(generator());
  std::string status_channel = boost::uuids::to_string(generator());
  std::string worker_channel = boost::uuids::to_string(generator());

  //Create a second server, and make client and workers based on it
  //this will verify that the server has zmq thread management, and that we
  //can have multiple server bound from the same instance
  zmq::socketInfo<zmq::proto::inproc> ci(client_channel);
  zmq::socketInfo<zmq::proto::inproc> si(status_channel);
  zmq::socketInfo<zmq::proto::inproc> wi(worker_channel);
  remus::server::ServerPorts inproc_ports(ci,si,wi);

  boost::shared_ptr<remus::Server> inproc_server( new remus::Server(inproc_ports) );
  inproc_server->startBrokering();

  boost::shared_ptr<remus::Client> in_procclient = detail::make_Client( inproc_ports, true );
  boost::shared_ptr<remus::Worker> in_procworker = detail::make_Worker( inproc_ports, io_type, "SimpleWorker", true );

  swap_worker_connection( in_procworker );
  swap_client_connection( in_procclient );

  return 0;
}
