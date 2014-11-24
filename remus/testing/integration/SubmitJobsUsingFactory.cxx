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

  static std::size_t small_size = 1024;
  static std::size_t large_size = 16777216;

  static std::size_t ascii_data_size = small_size;
  static std::size_t binary_data_size = small_size;

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
remus::proto::JobRequirements make_Reqs()
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements requirements = make_JobRequirements(io_type, "SimpleWorker", "");
  return requirements;
}

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports,
                                              std::size_t num_workers)
{
  boost::shared_ptr<factory::ThreadPoolWorkerFactory> factory(
            new factory::ThreadPoolWorkerFactory(make_Reqs(), num_workers));

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
remus::proto::Job submit_Job(boost::shared_ptr<remus::Client> client,
                             remus::proto::JobContent binary_content)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  std::stringstream ascii_data_size_buffer;
  ascii_data_size_buffer << ascii_data_size;

  JobContent asciiLength = JobContent(remus::common::ContentFormat::User,
                                      ascii_data_size_buffer.str());

  JobSubmission sub( make_Reqs() );
  sub["binary"] = binary_content;
  sub["ascii_data_size"] = asciiLength;
  remus::proto::Job job = client->submitJob(sub);
  REMUS_ASSERT(job.valid())
  return job;
}

//------------------------------------------------------------------------------
bool verify_jobs(boost::shared_ptr<remus::Client> client,
                 std::size_t num_jobs_to_submit)
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
  unsigned int num_valid_finished_jobs = 0;
  while(jobs.size() > 0)
    {
    //remove if moves all bad items to end of the vector and returns
    //an iterator to the new end. Remove if is easiest way to remove from middle
    FinishedOrFailed fofJobs(client.get(), &num_valid_finished_jobs);
    //You can't access the elements that remove_if shuffled as they aren't
    //required to be valid
    jobs.erase(std::remove_if(jobs.begin(),jobs.end(),fofJobs),
               jobs.end());
    }

  //return true when the number of jobs that finished properly matches
  //the number of jobs submitted. We don't use REMUS_ASSERT in this
  //thread, but instead
  return num_valid_finished_jobs == num_jobs_to_submit;
}

}

//
int main(int argc, char* argv[])
{
  //if no parameters just run with a single worker and single job
  std::size_t num_workers = 1;
  std::size_t num_jobs = 1;
  std::string data_size_flag = "small";
  if( argc == 4)
    {
    std::stringstream buffer;
    buffer << argv[1] << std::endl;
    buffer << argv[2] << std::endl;
    buffer << argv[3] << std::endl;
    buffer >> data_size_flag;
    buffer >> num_workers;
    buffer >> num_jobs;
    }

  if(data_size_flag == "large")
    {
    ascii_data_size = large_size;
    binary_data_size = large_size;
    }

  std::cout << "verifying " << num_workers << " workers "
            << num_jobs << " jobs" << std::endl;

  //construct a threaded pool factory
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts(),
                                                         num_workers );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  //construct the client interface
  boost::shared_ptr<remus::Client> client = make_Client( ports );

  const bool valid = verify_jobs(client, num_jobs);

  //now to cleanup the workers we tell the server to stop brokering
  //which will tell the workers to stop running
  server->stopBrokering();

  //now validate that all workers received a fair share of the jobs

  //now verify that all the jobs finished properly
  return ( valid == true ) ? 0 : 1;
}
