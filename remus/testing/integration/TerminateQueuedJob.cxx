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
#include <remus/testing/integration/detail/Factories.h>
#include <remus/testing/integration/detail/Helpers.h>

namespace
{
  namespace detail
  {
  using namespace remus::testing::integration::detail;
  }

  //global store of data to verify on worker
  namespace data
  {
  std::string ascii_data;
  std::string binary_data;
  }

//------------------------------------------------------------------------------
std::vector< remus::common::MeshIOType > make_all_meshTypes()
{
  using namespace remus::meshtypes;
  using namespace remus::common;

  typedef boost::shared_ptr< MeshTypeBase > MeshType;
  std::set< MeshType > allRegTypes =
                    remus::common::MeshRegistrar::allRegisteredTypes();

  std::vector< MeshIOType > allTypes(allRegTypes.size());
  typedef std::set<MeshType>::const_iterator cit;
  for( cit i = allRegTypes.begin(); i != allRegTypes.end(); ++i)
    {
    for(cit j = allRegTypes.begin(); j != allRegTypes.end(); ++j)
      {
      MeshIOType io_type( (*i)->name(), (*j)->name());
      allTypes.push_back(io_type);
      }
    }

  return allTypes;
}

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  //create the server and start brokering, with an empty factory
  boost::shared_ptr<detail::AlwaysSupportFactory> factory(new detail::AlwaysSupportFactory("AlwaysSupportWorker"));
  factory->setMaxWorkerCount(1); //max worker needs to be higher than 0
  boost::shared_ptr<remus::Server> server( new remus::Server(ports,factory) );
  server->startBrokering();
  return server;
}

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Client> make_Client( const remus::server::ServerPorts& ports )
{
  remus::client::ServerConnection conn =
              remus::client::make_ServerConnection(ports.client().endpoint());

  boost::shared_ptr<remus::Client> c(new remus::client::Client(conn));
  return c;
}

//------------------------------------------------------------------------------
void verify_can_mesh(boost::shared_ptr<remus::Client> client)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  std::vector<remus::common::MeshIOType> ioTypes = make_all_meshTypes();
  typedef std::vector<remus::common::MeshIOType>::const_iterator it;
  for(it i=ioTypes.begin(); i!=ioTypes.end();++i)
    {
    //first verify simple can mesh
    const bool can_mesh = client->canMesh(*i);

    //second verify can mesh given requirements
    JobRequirements fakeReqs = make_JobRequirements(*i,"","");
    const bool can_mesh_reqs = client->canMesh(fakeReqs);

    //lastly verify we get requirements back from retrieveRequirements
    remus::proto::JobRequirementsSet reqs = client->retrieveRequirements(*i);

    REMUS_ASSERT( can_mesh )
    REMUS_ASSERT( can_mesh_reqs )
    REMUS_ASSERT( (reqs.size()>0) )
    }
}

//------------------------------------------------------------------------------
remus::proto::Job verify_job_submission(boost::shared_ptr<remus::Client> client)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements reqs = make_JobRequirements(io_type, "SimpleWorker", "");

  //save this data to global variables so we can check them in the worker
  data::ascii_data = remus::testing::AsciiStringGenerator(2097152);
  data::binary_data = remus::testing::BinaryDataGenerator(8388608);

  JobContent canary_data = make_JobContent("canary");
  JobContent random_ascii_data = make_JobContent( data::ascii_data );
  JobContent random_binary_data = make_JobContent( data::binary_data);

  JobSubmission sub(reqs);

  //submit a job with some random data, we will check only canary
  sub["canary"]=canary_data;
  sub["ascii"]=random_ascii_data;
  sub["binary"]=random_binary_data;
  remus::proto::Job job = client->submitJob(sub);
  REMUS_ASSERT(job.valid())
  return job;
}

//------------------------------------------------------------------------------
void verify_job_result(remus::proto::Job  job,
                       boost::shared_ptr<remus::Client> client)
{

  remus::proto::JobResult results = client->retrieveResults(job);
  REMUS_ASSERT( (results.valid()==false) )
  detail::verify_job_status(job,client,remus::QUEUED);
}

//------------------------------------------------------------------------------
void verify_terminate_job(remus::proto::Job  job,
                          boost::shared_ptr<remus::Client> client)
{
  remus::proto::JobStatus ts = client->terminate(job);
  REMUS_ASSERT( (ts.failed()) )

  ts = client->terminate(job); //cant terminate twice
  REMUS_ASSERT( (ts.invalid()) )
}

}

int TerminateQueuedJob(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  //construct a simple worker and client
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );

  //verify that the servers says that we can mesh without any workers connected
  verify_can_mesh(client);

  //verify that we can submit jobs to the server without any workers
  remus::proto::Job job = verify_job_submission(client);
  detail::verify_job_status(job,client,remus::QUEUED);

  //try to get results from a job that hasn't finished
  verify_job_result(job,client);

  //no terminate the job that is sitting on the server
  verify_terminate_job(job,client);

  //verify that you can't get back valid status for a terminated job
  detail::verify_job_status(job,client,remus::INVALID_STATUS);

  return 0;
}
