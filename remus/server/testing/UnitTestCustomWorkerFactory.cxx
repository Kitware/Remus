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

#include <remus/server/detail/WorkerPool.h>

#include <iostream>

namespace {

using namespace remus::common;
using namespace remus::meshtypes;


//we want a custom factory that acts like it can create workers when queried
//but actually doesn't when asked too. This will allow the server to queue
//jobs when no worker is connected, instead of rejecting all jobs that are
//submitted
class DoNothingFactory: public remus::server::WorkerFactory
{
public:
  remus::proto::JobRequirementsSet workerRequirements(
                                          remus::common::MeshIOType type) const
  {
    //return a fake set of requirements which makes it look like we can mesh
    //the given input & output type.
    remus::proto::JobRequirementsSet result;
    remus::proto::JobRequirements reqs =
        remus::proto::make_JobRequirements(type, std::string(), std::string());
    result.insert(reqs);
    return result;
  }

  bool haveSupport(const remus::proto::JobRequirements& reqs) const
    {
    (void)reqs;
    //we want to return true here so that the server always queues
    return true;
    }

  bool createWorker(const remus::proto::JobRequirements& type,
                    WorkerFactory::FactoryDeletionBehavior lifespan)
    {
    (void) type;
    (void) lifespan;
    //we want to return false here so that server never thinks we are creating
    //a worker and assigns a job to a worker we didn't create
    return false;
    }
};

//wrapper around server to allow us to check the state of the WorkerFactory,
//since that is not possible with the default server.
class DerivedServer : public remus::server::Server
{
public:
  explicit DerivedServer(
     const boost::shared_ptr<remus::server::WorkerFactory>& factory):
    remus::server::Server(factory)
  {

  }

  bool check_FactorySupport(remus::proto::JobRequirements reqs) const
  {
    const bool support =
      (this->WorkerFactory->workerRequirements(reqs.meshTypes()).size() > 0) &&
      (this->WorkerFactory->maxWorkerCount() > 0);
    return support;
  }

  bool check_FactoryCreateWorker(remus::proto::JobRequirements reqs)
  {
    return this->WorkerFactory->createWorker(reqs,
                          remus::server::WorkerFactory::KillOnFactoryDeletion);
  }

};

template<typename In, typename Out>
remus::proto::JobRequirements make_Reqs(In in, Out out)
{
  return remus::proto::make_JobRequirements( MeshIOType(in,out),
                                             std::string(),
                                             std::string());
}

void test_derived_factory()
{
  DoNothingFactory factory;

  //verify that we state we support any type
  remus::proto::JobRequirements edges_twod = make_Reqs(Edges(),Mesh2D());
  remus::proto::JobRequirements edges_edges = make_Reqs(Edges(),Edges());
  remus::proto::JobRequirements twod_twod = make_Reqs(Mesh2D(),Mesh2D());

  REMUS_ASSERT( (factory.haveSupport(edges_twod)) );
  REMUS_ASSERT( (factory.haveSupport(edges_edges)) );
  REMUS_ASSERT( (factory.haveSupport(twod_twod)) );

  REMUS_ASSERT( (factory.maxWorkerCount() == 1) );

  REMUS_ASSERT( (factory.currentWorkerCount() == 0) );
  REMUS_ASSERT( (factory.currentWorkerCount() <  factory.maxWorkerCount()) );

  //verify that updateWorkerCount changes nothing.
  factory.updateWorkerCount();
  REMUS_ASSERT( (factory.currentWorkerCount() == 0) );
  REMUS_ASSERT( (factory.currentWorkerCount() <  factory.maxWorkerCount()) );


  //setup the behavior for the lifespan of the workers.
  const remus::server::WorkerFactory::FactoryDeletionBehavior kill =
                remus::server::WorkerFactory::KillOnFactoryDeletion;
  REMUS_ASSERT( (factory.createWorker(edges_twod,kill) == false) );
  REMUS_ASSERT( (factory.createWorker(edges_edges,kill) == false) );
  REMUS_ASSERT( (factory.createWorker(twod_twod,kill) == false) );

  //make sure we haven't incremented our worker count
  REMUS_ASSERT( (factory.currentWorkerCount() == 0) );

  // verify again updateWorkerCount does nothing.
  factory.updateWorkerCount();
  REMUS_ASSERT( (factory.currentWorkerCount() == 0) );
  REMUS_ASSERT( (factory.currentWorkerCount() <  factory.maxWorkerCount()) );
}

void test_server_using_derived_factory()
{
  boost::shared_ptr<DoNothingFactory> factory(new DoNothingFactory());
  DerivedServer server_def(factory);

  server_def.startBrokering();

  remus::proto::JobRequirements edges_twod = make_Reqs(Edges(),Mesh2D());
  remus::proto::JobRequirements edges_edges = make_Reqs(Edges(),Edges());
  remus::proto::JobRequirements twod_twod = make_Reqs(Mesh2D(),Mesh2D());

  REMUS_ASSERT( (server_def.check_FactorySupport(edges_twod)) );
  REMUS_ASSERT( (server_def.check_FactorySupport(edges_edges)) );
  REMUS_ASSERT( (server_def.check_FactorySupport(twod_twod)) );

  REMUS_ASSERT( (!server_def.check_FactoryCreateWorker(edges_twod)) );
  REMUS_ASSERT( (!server_def.check_FactoryCreateWorker(edges_edges)) );
  REMUS_ASSERT( (!server_def.check_FactoryCreateWorker(twod_twod)) );
}

} //namespace

int UnitTestCustomWorkerFactory(int, char *[])
{
  //Test server construction
  test_derived_factory();

  test_server_using_derived_factory();
  return 0;
}
