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

namespace
{
  //global store of data to verify on worker
  namespace data
  {
  std::string ascii_data;
  std::string binary_data;
  }

  namespace factory
  {
  //we want a custom factory that can not create any workers
  //but alays states that it can support a mesh type
  class AlwaysSupportFactory: public remus::server::WorkerFactoryBase
  {
  public:

    remus::common::MeshIOTypeSet supportedIOTypes() const
    {
      //we need to return that we support all types!
      return remus::common::generateAllIOTypes();
    }

    remus::proto::JobRequirementsSet workerRequirements(
                                            remus::common::MeshIOType type) const
    {
      //make clients think we have real workers, by sending back fake job reqs
      remus::proto::JobRequirements reqs =
           remus::proto::make_JobRequirements(type,"SimpleWorker","");
      remus::proto::JobRequirementsSet reqSet;
      reqSet.insert(reqs);
      return reqSet;
    }

    bool haveSupport(const remus::proto::JobRequirements& reqs) const
      {
      (void) reqs;
      //we want to return true so that the server adds jobs to it worker queue
      return true;
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

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  //create the server and start brokering, with an empty factory
  boost::shared_ptr<factory::AlwaysSupportFactory> factory(new factory::AlwaysSupportFactory());
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
boost::shared_ptr<remus::Worker> make_Worker( const remus::server::ServerPorts& ports )
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::worker::ServerConnection conn =
              remus::worker::make_ServerConnection(ports.worker().endpoint());

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements requirements = make_JobRequirements(io_type, "SimpleWorker", "");
  boost::shared_ptr<remus::Worker> w(new remus::Worker(requirements,conn));
  return w;
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
  (void) argc;
  (void) argv;

  //construct a simple worker and client
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );
  boost::shared_ptr<remus::Worker> worker = make_Worker( ports );

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
