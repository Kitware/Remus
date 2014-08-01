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

namespace factory
{
//we want a custom factory that can not create any workers
//but alays states that it can support a mesh type
class AlwaysSupportFactory: public remus::server::WorkerFactory
{
public:
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

namespace
{
  //global store of data to verify on worker
  namespace data
  {
  std::string ascii_data;
  std::string binary_data;
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

  //what really we need are iterators to the mesh registrar
  const std::size_t num_mesh_types = MeshRegistrar::numberOfRegisteredTypes();

  std::vector< MeshIOType > allTypes;
  for(std::size_t input_type = 1; input_type < num_mesh_types+1; ++input_type)
    {
    for(std::size_t output_type = 1; output_type < num_mesh_types+1; ++output_type)
      {
      MeshIOType io_type( to_meshType(input_type), to_meshType(output_type));
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
  conn.context(ports.context());

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
  conn.context(ports.context());

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
void verify_job_result(remus::proto::Job  job,
                       boost::shared_ptr<remus::Client> client)
{

  remus::proto::JobResult results = client->retrieveResults(job);
  REMUS_ASSERT( (results.valid()==false) )
  verify_job_status(job,client,remus::QUEUED);
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
  //construct a simple worker and client, we need to share the same
  //context between the server, client and worker
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );

  //verify that the servers says that we can mesh without any workers connected
  verify_can_mesh(client);

  //verify that we can submit jobs to the server without any workers
  remus::proto::Job job = verify_job_submission(client);
  verify_job_status(job,client,remus::QUEUED);

  //try to get results from a job that hasn't finished
  verify_job_result(job,client);

  //no terminate the job that is sitting on the server
  verify_terminate_job(job,client);

  //verify that you can't get back valid status for a terminated job
  verify_job_status(job,client,remus::INVALID_STATUS);

  return 0;
}
