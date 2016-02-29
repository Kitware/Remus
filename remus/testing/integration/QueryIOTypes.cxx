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
#include <remus/server/WorkerFactoryBase.h>
#include <remus/worker/Worker.h>

//required to use custom contexts
#include <remus/proto/zmq.hpp>

#include <remus/common/SleepFor.h>
#include <remus/testing/Testing.h>
#include <remus/testing/integration/detail/Helpers.h>

#include <utility>

namespace factory
{
//custom factory that support a single mesh io type
class BridgeFactory: public remus::server::WorkerFactoryBase
{
public:

  remus::common::MeshIOTypeSet supportedIOTypes() const
    {
    //we need to say we only support a single type
    remus::common::MeshIOTypeSet validTypes;
    validTypes.insert( remus::common::MeshIOType("Model", "Bridge") );
    return validTypes;
    }

  remus::proto::JobRequirementsSet workerRequirements(
                                            remus::common::MeshIOType type) const
    {
    (void) type;
    remus::proto::JobRequirementsSet reqSet;
    return reqSet;
    }

  bool haveSupport(const remus::proto::JobRequirements& reqs) const
    {
    (void) reqs;
    //we want to return false so that the server requires a worker
    //to be connected to add jobs
    return false;
    }

  bool createWorker(const remus::proto::JobRequirements& type,
                    WorkerFactoryBase::FactoryDeletionBehavior lifespan)
    {
    (void) type;
    (void) lifespan;
    //we want to return false here so that server never thinks we are creating
    //a worker and assigns a job to a worker we didn't create
    return false;
    }

  void updateWorkerCount(){}
  unsigned int currentWorkerCount() const { return 0; }

};
}

namespace
{
  namespace detail
  {
  using namespace remus::testing::integration::detail;
  }

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
  using namespace remus::meshtypes;

  (void) argc;
  (void) argv;

  //construct multiple simple workers and a client, we need to share the same
  //context between the server, client and worker
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = detail::make_Client( ports );

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
    remus::common::MeshIOType io_type("Model",output_types[i]);
    handles.push_back( detail::make_Worker( ports, io_type, "SimpleWorker", true ) );
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
