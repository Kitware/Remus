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
#include <remus/testing/integration/detail/Factories.h>
#include <remus/testing/integration/detail/Workers.h>


#include <utility>

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <boost/thread.hpp>

#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif
namespace
{
  namespace factory
  {
  using remus::testing::integration::detail::ThreadPoolWorkerFactory;
  }
  static std::size_t ascii_data_size = 1024;
  static std::size_t binary_data_size = 1024;

//------------------------------------------------------------------------------
struct FinishedOrFailed
{
  remus::Client* client;
  unsigned int* num_finished_jobs;
  FinishedOrFailed(remus::Client* c, unsigned int* numFinJobs):
    client(c),
    num_finished_jobs(numFinJobs)
    {

    }

  inline bool operator()(const remus::proto::Job& job)
    {
    remus::proto::JobStatus st = client->jobStatus(job);
    if(st.finished())
      {
      remus::proto::JobResult r = client->retrieveResults( job );
      if( r.dataSize() > 0 )
        { ++(*this->num_finished_jobs); }
      }
    return st.finished() || st.failed();
    }
};

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Client> make_Client( const remus::server::ServerPorts& ports )
{
  remus::client::ServerConnection conn =
              remus::client::make_ServerConnection(ports.client().endpoint());

  boost::shared_ptr<remus::Client> c(new remus::client::Client(conn));
  return c;
}

//------------------------------------------------------------------------------
remus::proto::Job submit_Job(boost::shared_ptr<remus::Client> client,
                             remus::proto::JobContent binary_content)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements reqs = make_JobRequirements(io_type, "SimpleWorker", "");

  JobSubmission sub(reqs);
  sub["binary"]=binary_content;

  std::stringstream ascii_data_size_buffer;
  ascii_data_size_buffer << ascii_data_size;

  JobContent asciiLength = JobContent(remus::common::ContentFormat::User,
                                      ascii_data_size_buffer.str());
  sub["ascii_data_size"] = asciiLength;

  remus::proto::Job job = client->submitJob(sub);
  REMUS_ASSERT(job.valid())
  return job;
}

//------------------------------------------------------------------------------
void terminate_blocking_workers(boost::shared_ptr<remus::Client> client,
                                boost::shared_ptr<remus::Server> server,
                                std::size_t num_jobs_to_submit,
                                std::size_t num_jobs_to_terminate_at)
{
  using namespace remus::proto;

  //compute the binary data once, and than make a JobContent that is a zero copy
  //to keep the memory footprint low so we don't cause OOM issues on Travis
  //dashboards.
  const std::string binary_input = remus::testing::BinaryDataGenerator( binary_data_size );
  JobContent random_binary_content = JobContent(remus::common::ContentFormat::User,
                                                binary_input.c_str(),
                                                binary_input.size() );

  //submit each job to the server, we know that the we have workers
  //attached to the server by this point so all jobs will be accepted
  std::vector< remus::proto::Job > jobs;
  std::cout << "submitting jobs" << std::endl;
  for( std::size_t i = 0; i < num_jobs_to_submit; ++i)
    {
    Job job_i = submit_Job( client, random_binary_content );
    jobs.push_back( job_i );
    }

  //query the server for the job completion
  std::cout << "querying job status" << std::endl;
  unsigned int num_finished_jobs = 0;
  while(jobs.size() > 0)
    {
    //remove if moves all bad items to end of the vector and returns
    //an iterator to the new end. Remove if is easiest way to remove from middle
    FinishedOrFailed fofJobs(client.get(), &num_finished_jobs);

    //after the call to remove_if accessing the elements is not allowed
    //as they don't have to be valid
    jobs.erase(std::remove_if(jobs.begin(),jobs.end(),fofJobs),
               jobs.end());
    if(num_finished_jobs >= num_jobs_to_terminate_at)
      {
      std::cout << "stopBrokering" << std::endl;
      jobs.clear();
      server->stopBrokering();
      }
    }
    std::cout << "num_finished_jobs: " << num_finished_jobs << std::endl;
}

}

//
int TerminateMultipleRunningWorkers(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  using namespace remus::meshtypes;
  using namespace remus::proto;

  //if no parameters just run with a single worker and single job
  std::size_t num_workers = 7;
  std::size_t num_jobs = 100;
  std::size_t num_jobs_to_terminate_at = 18;

  //worker requirments
  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements requirements = make_JobRequirements(io_type, "SimpleWorker", "");

  //create the server and start brokering, with a worker pool factory
  //that will manage the fake workers
  boost::shared_ptr<factory::ThreadPoolWorkerFactory> factory(
            new factory::ThreadPoolWorkerFactory(requirements, num_workers));

  boost::shared_ptr<remus::Server> server( new remus::Server(remus::server::ServerPorts(),factory) );
  server->startBrokering();

  //construct a simple client
  const remus::server::ServerPorts& ports = server->serverPortInfo();
  boost::shared_ptr<remus::Client> client = make_Client( ports );

  std::cout << "verifying "
            << num_workers << " workers "
            << num_jobs << " jobs" << std::endl;

  terminate_blocking_workers(client, server, num_jobs, num_jobs_to_terminate_at);

  return 0;
}
