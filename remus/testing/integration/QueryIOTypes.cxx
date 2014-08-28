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
#include <remus/server/WorkerFactory.h>
#include <remus/worker/Worker.h>

//required to use custom contexts
#include <remus/proto/zmq.hpp>

#include <remus/common/SleepFor.h>
#include <remus/testing/Testing.h>

#include <utility>

namespace factory
{
//custom factory that support a single mesh io type
class BridgeFactory: public remus::server::WorkerFactory
{
public:

  remus::common::MeshIOTypeSet supportedIOTypes() const
  {
    //we need to say we only support a single type
    remus::common::MeshIOTypeSet validTypes;
    validTypes.insert( remus::common::MeshIOType("Model", "Bridge") );
    return validTypes;
  }
};
}

namespace
{
//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  //create the server and start brokering, with a factory that can launch
  //no workers, so we have to use workers that connect in only
  boost::shared_ptr<factory::BridgeFactory> factory(new factory::BridgeFactory());
  factory->setMaxWorkerCount(0);
  boost::shared_ptr<remus::Server> server( new remus::Server(ports,factory) );
  server->startBrokering();
  return server;
}

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Client> make_Client( const remus::server::ServerPorts& ports )
{
  remus::client::ServerConnection conn =
              remus::client::make_ServerConnection(ports.client().endpoint());
  conn.context(ports.context());

  boost::shared_ptr<remus::Client> c(new remus::client::Client(conn));
  return c;
}

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Worker> make_Worker(const remus::server::ServerPorts& ports,
                                             std::string input_type,
                                             std::string output_type )
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::worker::ServerConnection conn =
              remus::worker::make_ServerConnection(ports.worker().endpoint());
  conn.context(ports.context());

  remus::common::MeshIOType io_type(input_type,output_type);
  JobRequirements requirements = make_JobRequirements(io_type, "SimpleWorker", "");
  boost::shared_ptr<remus::Worker> w(new remus::Worker(requirements,conn));
  return w;
}

//------------------------------------------------------------------------------
void verify_supportsIOType(boost::shared_ptr<remus::Client> client,
                           std::string output_type)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  //ask the client to get all the valid types that the server knows about
  remus::common::MeshIOTypeSet validTypes = client->supportedIOTypes();
  REMUS_ASSERT( (validTypes.size() >= 1) );

  remus::common::MeshIOType expected_to_support("Model", output_type);
  REMUS_ASSERT( (validTypes.count(expected_to_support) == 1) );

  remus::common::MeshIOType not_supported(output_type, "Model");
  REMUS_ASSERT( (validTypes.count(not_supported) == 0) );
}

}

//Constructs workers that use new mesh types, and verifies that the client
//can retrieve those types from the server
int QueryIOTypes(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  //construct multiple simple workers and a client, we need to share the same
  //context between the server, client and worker
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );

  //verify right now that the server MeshIOType of Model:Bridge which was
  //part of the factory
  verify_supportsIOType(client, "Bridge" );

  typedef boost::shared_ptr<remus::Worker> WorkerHandle;
  std::vector< WorkerHandle > handles;

  std::vector<std::string> output_types;
  output_types.push_back("BridgeA");
  output_types.push_back("BridgeB");
  output_types.push_back("BridgeC");

  for(std::size_t i=0; i <3; ++i)
    {
    handles.push_back( make_Worker( ports, "Model", output_types[i] ) );
    handles[i]->askForJobs(1);
    }
  remus::common::SleepForMillisec(35);

  //now that everything is up and running, verify that the client can
  //detect all the newly registered meshIOtypes
  for(std::size_t i=0; i <3; ++i)
    {
    verify_supportsIOType(client, output_types[i] );
    }

  return 0;
}
