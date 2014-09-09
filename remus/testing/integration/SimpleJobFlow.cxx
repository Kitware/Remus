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

namespace
{

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  //create the server and start brokering, with a factory that can launch
  //no workers, so we have to use workers that connect in only
  boost::shared_ptr<remus::server::WorkerFactory> factory(new remus::server::WorkerFactory());
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
void verify_job_status(remus::proto::Job  job,
                       boost::shared_ptr<remus::Client> client,
                       remus::STATUS_TYPE statusType)
{
  using namespace remus::proto;

  remus::common::SleepForMillisec(25);
  JobStatus currentStatus = client->jobStatus(job);
  const bool valid_status = (currentStatus.status() == statusType);
  REMUS_ASSERT(valid_status)
}


//------------------------------------------------------------------------------
void verify_can_mesh(boost::shared_ptr<remus::Client> client,
                     boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType bad_IOType( (Mesh3D()), (Mesh2D()) );
  REMUS_ASSERT( (client->canMesh(bad_IOType) == false) );

  remus::common::MeshIOType good_IOType( (Mesh2D()), (Mesh3D()) );
  REMUS_ASSERT( (client->canMesh(good_IOType) == false) );

  //ask for a job on the worker, and wait for server to process the
  //request
  worker->askForJobs(1);
  remus::common::SleepForMillisec(50);

  //now that the server knows a worker can handle a job of the given type
  //make the client verifies that it can mesh that job type
  REMUS_ASSERT( (client->canMesh(good_IOType) == true) );
}

//------------------------------------------------------------------------------
remus::proto::Job verify_job_submission(boost::shared_ptr<remus::Client> client,
                                        boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::proto;
  using namespace remus::meshtypes;

  //verify no jobs pending before we submit
  const std::size_t pre_numPendingJobs = worker->pendingJobCount();
  REMUS_ASSERT( (pre_numPendingJobs==0) )

  //get back from the server all the workers that match our input/output type
  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirementsSet reqsFromServer = client->retrieveRequirements(io_type);
  REMUS_ASSERT( (reqsFromServer.size()==1) )

  //craft a submission using the first workers reqs
  JobSubmission sub((*reqsFromServer.begin()));
  sub["extra_stuff"] = make_JobContent("random data");

  //now submit that job
  Job clientJob = client->submitJob(sub);
  REMUS_ASSERT( clientJob.valid() )

  //verify the status of the job
  verify_job_status(clientJob,client,remus::QUEUED);

  //wait for the server to send the job to the worker
  remus::common::SleepForMillisec(50);
  const std::size_t numPendingJobs = worker->pendingJobCount();
  REMUS_ASSERT( (numPendingJobs==1) )

  remus::worker::Job workerJob = worker->takePendingJob();
  REMUS_ASSERT(workerJob.valid())

  //verify the content of the job on the worker
  const JobSubmission& workerSub = workerJob.submission();

  //this will verify each has the same keys and values to the keys
  REMUS_ASSERT( (workerSub == sub) )

  { //sanity check to show that == work with JobSubmission
  JobSubmission sub2((*reqsFromServer.begin()));
  sub2["extra_stuff"] = make_JobContent("random data not the same");
  REMUS_ASSERT( (!(workerSub == sub2)) )
  }

  return clientJob;
}

//------------------------------------------------------------------------------
void verify_job_processing(const remus::proto::Job& job,
                       boost::shared_ptr<remus::Client> client,
                       boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::proto;

  //create a job progress to send to the client
  JobProgress progress(50);
  JobStatus workerStatus(job.id(), progress);
  worker->updateStatus(workerStatus);

  verify_job_status(job,client,remus::IN_PROGRESS);

  //grab the status on the client and verify it matches the status we sent
  //for the worker
  JobStatus clientStatus = client->jobStatus(job);
  REMUS_ASSERT( (clientStatus==workerStatus) )

}

//------------------------------------------------------------------------------
void verifyt_job_result(const remus::proto::Job& job,
                        boost::shared_ptr<remus::Client> client,
                        boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::proto;

  const std::string ascii_data = remus::testing::AsciiStringGenerator(2097152);

  JobResult worker_results = make_JobResult(job.id(),ascii_data);
  worker->returnMeshResults(worker_results);

  remus::common::SleepForMillisec(50);

  //after the job result has been submitted back to the server
  //the status should be finished
  verify_job_status(job,client,remus::FINISHED);

  remus::proto::JobResult client_results = client->retrieveResults(job);
  REMUS_ASSERT( (client_results.valid()==true) )

  const std::string resultText(client_results.data());
  REMUS_ASSERT( (resultText==ascii_data) )

}

}

//Constructs a job in the simplist way possible and
//verifies that it the worker runs and gets results
int SimpleJobFlow(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  //construct a simple worker and client
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );
  boost::shared_ptr<remus::Worker> worker = make_Worker( ports );

  //now that everything is up and running verify that the simple
  //submit,query status, get results logic flow works properly
  verify_can_mesh(client,worker);
  remus::proto::Job job = verify_job_submission(client,worker);
  verify_job_processing(job,client,worker);
  verifyt_job_result(job,client,worker);

  return 0;
}
