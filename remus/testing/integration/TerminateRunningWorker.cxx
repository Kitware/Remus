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
  std::string binary_data;
  }

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  //create the server and start brokering, with an empty factory
  boost::shared_ptr<detail::AlwaysSupportFactory> factory(new detail::AlwaysSupportFactory("SimpleWorker"));
  factory->setMaxWorkerCount(1); //max worker needs to be higher than 0
  boost::shared_ptr<remus::Server> server( new remus::Server(ports,factory) );
  server->startBrokering();
  return server;
}

//------------------------------------------------------------------------------
remus::proto::Job submit_Job(boost::shared_ptr<remus::Client> client)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements reqs = make_JobRequirements(io_type, "SimpleWorker", "");

  //compute a small binary data blob to send as the fake job data, and use
  //the zero copy JobContent constructor
  const std::string binary_input = remus::testing::BinaryDataGenerator( 524288 );
  JobContent random_binary_content = JobContent(remus::common::ContentFormat::User,
                                                binary_input.c_str(),
                                                binary_input.size() );


  JobSubmission sub(reqs);
  sub["binary"]=random_binary_content;
  remus::proto::Job job = client->submitJob(sub);
  REMUS_ASSERT(job.valid())
  return job;
}

//------------------------------------------------------------------------------
remus::worker::Job take_job(boost::shared_ptr<remus::Worker> worker)
{
  //wait for the server to send the job to the worker
  worker->askForJobs();
  std::size_t numPendingJobs = worker->pendingJobCount();
  while(numPendingJobs == 0)
    {
    remus::common::SleepForMillisec(50);
    numPendingJobs = worker->pendingJobCount();
    }
  REMUS_ASSERT( (numPendingJobs==1) )

  remus::worker::Job workerJob = worker->takePendingJob();
  REMUS_ASSERT(workerJob.valid())

  //verify that we aren't stating the job should be terminated just after
  //taking it
  REMUS_ASSERT( (worker->jobShouldBeTerminated( workerJob ) == false) )

  return workerJob;
}

//------------------------------------------------------------------------------
void terminate_server(boost::shared_ptr<remus::Server> server)
{
  server->stopBrokering();
  if(server->isBrokering())
    {
    server->waitForBrokeringToFinish();
    }
}

//------------------------------------------------------------------------------
void verify_worker_is_terminated(boost::shared_ptr<remus::Worker> worker)
{
  //wait for the server to send terminate worker msg
  while( !worker->workerShouldTerminate( ) )
    { remus::common::SleepForMillisec(50); }

  const std::size_t numPendingJobs = worker->pendingJobCount();
  REMUS_ASSERT( (numPendingJobs==1) )

  remus::worker::Job workerJob = worker->takePendingJob();
  REMUS_ASSERT( (workerJob.valid() == false) )
  REMUS_ASSERT( (workerJob.validityReason() == remus::worker::Job::TERMINATE_WORKER) )
}


}

int TerminateRunningWorker(int argc, char* argv[])
{
  using namespace remus::meshtypes;
  (void) argc;
  (void) argv;

  //construct a simple worker and client
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  boost::shared_ptr<remus::Client> client = detail::make_Client( ports );
  boost::shared_ptr<remus::Worker> worker = detail::make_Worker( ports, io_type, "SimpleWorker" );

  //have the client submit the job
  submit_Job(client);

  //have the worker pull the job
  remus::worker::Job workerJob = take_job(worker);

  //now the job is 'active' have the server stop brokering call terminate_server
  //which will tell the workers they need to shutdown
  terminate_server(server);

  //verify that the worker API states the job should be terminated
  //verify that the worker has a pending job which is a "terminate job" job
  verify_worker_is_terminated(worker);
  return 0;
}
