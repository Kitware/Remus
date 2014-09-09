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
  class AlwaysSupportFactory: public remus::server::WorkerFactory
  {
  public:

    //when asked about which IOTypes we support we say none
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
           remus::proto::make_JobRequirements(type,"AlwaysSupportWorker","");
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
                      WorkerFactory::FactoryDeletionBehavior lifespan)
      {
      (void) type;
      (void) lifespan;
      //we want to return false here so that server never thinks we are creating
      //a worker and assigns a job to a worker we didn't create
      return false;
      }

    };
  }


//------------------------------------------------------------------------------
bool get_value(const remus::proto::JobSubmission& data, const std::string& key,
               remus::proto::JobContent& value)
{
  typedef remus::proto::JobSubmission::const_iterator IteratorType;
  IteratorType attIt = data.find(key);
  if(attIt == data.end())
    {
    return false;
    }
  value = attIt->second;
  return true;
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
remus::worker::Job verify_worker_take_job(remus::proto::Job  job,
                                          boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::proto;

  remus::worker::Job wjob = worker->getJob();
  REMUS_ASSERT( (wjob.id() == job.id()) )

  //verify that the wjob is valid
  REMUS_ASSERT( (wjob.valid()) )

  //get the job submission
  const JobSubmission& sub = wjob.submission();
  REMUS_ASSERT( (sub.size() == 3) )

  //verify the binary and ascii data
  JobContent ascii, binary, canary;
  REMUS_ASSERT(get_value(sub,"ascii",ascii));
  REMUS_ASSERT(get_value(sub,"binary",binary));
  REMUS_ASSERT(get_value(sub,"canary",canary));

  const bool ascii_fine = (std::string(ascii.data(),ascii.dataSize()) == data::ascii_data);
  const bool binary_fine = (std::string(binary.data(),binary.dataSize()) == data::binary_data);
  const bool canary_fine = (std::string(canary.data(),canary.dataSize()) == "canary");
  REMUS_ASSERT(ascii_fine)
  REMUS_ASSERT(binary_fine)
  REMUS_ASSERT(canary_fine)

  return wjob;
}

//------------------------------------------------------------------------------
void verify_client_gets_correct_status(const remus::proto::Job& job,
                                       const remus::proto::JobStatus& workerStatus,
                                       boost::shared_ptr<remus::Client> client)
{
   using namespace remus::proto;
   remus::common::SleepForMillisec(25); //wait for server to get worker status
   const remus::proto::JobStatus clientStatus = client->jobStatus(job);
   REMUS_ASSERT( (clientStatus==workerStatus) )
}
//------------------------------------------------------------------------------
void verify_job_progress(const remus::proto::Job& job,
                         boost::shared_ptr<remus::Client> client,
                         boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::proto;

  //no progress from the worker yet, should still be queued
  verify_job_status(job,client,remus::QUEUED);

  {
  JobProgress workerProgress("starting work");
  JobStatus workerStatus(job.id(),workerProgress);
  worker->updateStatus(workerStatus);
  verify_client_gets_correct_status(job,workerStatus,client);
  }

  verify_job_status(job,client,remus::IN_PROGRESS);

  {
  //send another status message, and verify client side
  worker->updateStatus( ( JobStatus(job.id(),(JobProgress(25,"working"))) ) );
  verify_client_gets_correct_status(job,
        ( JobStatus(job.id(),(JobProgress(25,"working"))) ), client);
  }

  verify_job_status(job,client,remus::IN_PROGRESS);

  {
  //send another status message, and verify client side
  JobStatus nextStatus(job.id(),(JobProgress(250)) );
  worker->updateStatus(nextStatus);
  verify_client_gets_correct_status(job,nextStatus,client);
  }

  {
  //workers can't actually send back a finished status, that is only
  //possible when you send back the job result, so lets verify
  //that send back a status message of FINISHED is explicitly dropped
  //by the server
  JobStatus correctStatus(job.id(),(JobProgress(250)) );
  JobStatus badFinishedStatus(job.id(),remus::FINISHED);
  worker->updateStatus(badFinishedStatus);
  verify_client_gets_correct_status(job,correctStatus,client);
  }

  verify_job_status(job,client,remus::IN_PROGRESS);
}

//------------------------------------------------------------------------------
void verify_job_result(remus::proto::Job  job,
                       boost::shared_ptr<remus::Client> client,
                       boost::shared_ptr<remus::Worker> worker)
{
  using namespace remus::proto;

  JobResult worker_results = make_JobResult(job.id(),
                                     "Here be results");
  worker->returnMeshResults(worker_results);

  remus::common::SleepForMillisec(50);

  //after the job result has been submitted back to the server
  //the status should be finished
  verify_job_status(job,client,remus::FINISHED);


  remus::proto::JobResult client_results = client->retrieveResults(job);
  REMUS_ASSERT( (client_results.valid()==true) )

  const std::string resultText(client_results.data());
  REMUS_ASSERT( (resultText=="Here be results") )
}

}

//------------------------------------------------------------------------------
int AlwaysAcceptServer(int argc, char* argv[])
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
  verify_job_status(job,client,remus::QUEUED);

  //now create the worker
  boost::shared_ptr<remus::Worker> worker = make_Worker( ports );

  //verify the worker accepts the correct job
  verify_worker_take_job(job,worker);
  //since we haven't sent status from the worker the status
  //still should be QUEUED
  verify_job_status(job,client,remus::QUEUED);

  //verify that worker can send progress events and the client will
  //get them
  verify_job_progress(job,client,worker);
  verify_job_status(job,client,remus::IN_PROGRESS);

  //verify that the job result the worker sends is sent back to the
  //client properly
  verify_job_result(job,client,worker);

  //now job shouldn't exist on the server
  verify_job_status(job,client,remus::INVALID_STATUS);

  return 0;
}
