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
#include <remus/worker/Worker.h>

#include <remus/testing/Testing.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/date_time/posix_time/posix_time.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace
{

static std::size_t blob_size = 512;
static std::size_t num_messages = 1000000;

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Server> make_Server( remus::server::ServerPorts ports )
{
  boost::shared_ptr<remus::Server> server( new remus::Server(ports) );
  server->startBrokering();
  return server;
}

//------------------------------------------------------------------------------
boost::shared_ptr<remus::Client> make_Client( const remus::server::ServerPorts& ports )
{
  remus::client::ServerConnection conn =
              remus::client::make_ServerConnection(ports.client().endpoint());
  // conn.context(ports.context());

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

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Model(),Model());
  JobRequirements requirements = make_JobRequirements(io_type, "PerfWorker", "");
  boost::shared_ptr<remus::Worker> w(new remus::Worker(requirements,conn));
  return w;
}

//------------------------------------------------------------------------------
remus::proto::Job submit_Job(boost::shared_ptr<remus::Client> client)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Model(),Model());
  JobRequirements reqs = make_JobRequirements(io_type, "PerfWorker", "");

  JobSubmission sub(reqs);

  const std::string binary_input = remus::testing::BinaryDataGenerator( blob_size );
  sub["blob"] = JobContent(remus::common::ContentFormat::User, binary_input);

  remus::proto::Job job = client->submitJob(sub);
  REMUS_ASSERT(job.valid())
  return job;

}

//------------------------------------------------------------------------------
void client_query_performance(const std::vector<remus::proto::Job> jobs,
                              boost::shared_ptr< remus::Client > client)
{
  typedef boost::posix_time::ptime ptime;
  typedef boost::posix_time::time_duration time_duration;

  remus::proto::JobStatus status = client->jobStatus( jobs[0] );

  const ptime startTime = boost::posix_time::microsec_clock::local_time();

  for( std::size_t i=0; i < num_messages;)
    {
    for( std::vector<remus::proto::Job>::const_iterator job = jobs.begin();
        job != jobs.end() && i < num_messages;
        ++job, ++i)
      {
      //ask for the current status of a job
      status = client->jobStatus( *job );
      }
    }

  //Client is REQ/REP so everything has been sent/recv by this line
  const ptime endTime = boost::posix_time::microsec_clock::local_time();
  const time_duration dur = endTime - startTime;

  //sizeof char in c++ == 1, so length of string is size in bytes
  const boost::int64_t status_byte_size =
    static_cast<boost::int64_t>( remus::proto::to_string(status).size() );
  const boost::int64_t job_byte_size =
    static_cast<boost::int64_t>( remus::proto::to_string(jobs[0]).size() );

  const boost::int64_t total_bytes_sent = num_messages * job_byte_size;
  const boost::int64_t total_bytes_recv = num_messages * status_byte_size;

  std::cout << "Total bytes sent " <<  total_bytes_sent << " across " << num_messages << " messages." << std::endl;
  std::cout << "Total bytes recv " <<  total_bytes_recv << " across " << num_messages << " messages." << std::endl;

  const boost::int64_t message_pairs_per_msec = 2 * (num_messages/dur.total_milliseconds());
  std::cout << "Messages sent & recv per sec " << message_pairs_per_msec  << std::endl;

  const boost::int64_t bytes_sent_per_sec = 1000 * (total_bytes_sent / dur.total_milliseconds() );
  const boost::int64_t bytes_recv_per_sec = 1000 * (total_bytes_recv / dur.total_milliseconds() );

  const boost::int64_t kb_sent_per_sec = bytes_sent_per_sec / 1024;
  const boost::int64_t kb_recv_per_sec = bytes_recv_per_sec / 1024;
  std::cout << "Bandwidth sent per sec " << kb_sent_per_sec << " (KB/sec) "<< std::endl;
  std::cout << "Bandwidth recv per sec " << kb_recv_per_sec << " (KB/sec) "<< std::endl;

  //This is to make sure that our baseline performance doesn't degrade below
  //a certain level
#if defined(__APPLE__)
  REMUS_ASSERT( (kb_sent_per_sec >= 275))
  REMUS_ASSERT( (kb_recv_per_sec >= 275))
#elif defined(_WIN32)
  REMUS_ASSERT( (kb_sent_per_sec >= 275))
  REMUS_ASSERT( (kb_recv_per_sec >= 275))
#else
  //unix just does better with loopback
  REMUS_ASSERT( (kb_sent_per_sec >= 375))
  REMUS_ASSERT( (kb_recv_per_sec >= 375))
#endif


  //Okay, we have this test here, so in case we improve performance, that means
  //we can update the baseline performance. So if you start seeing this line
  //starting to fail, you should increase both our ceiling and floor for performance
#if defined(__APPLE__)
  REMUS_ASSERT( (kb_sent_per_sec <= 550))
  REMUS_ASSERT( (kb_recv_per_sec <= 550))
#elif defined(_WIN32)
  REMUS_ASSERT( (kb_sent_per_sec <= 550))
  REMUS_ASSERT( (kb_recv_per_sec <= 550))
#else
  //unix just does better with loopback
  REMUS_ASSERT( (kb_sent_per_sec <= 900))
  REMUS_ASSERT( (kb_recv_per_sec <= 900))
#endif

}

}

int main(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  //Construct multiple workers and a client to verify the performance
  //of the server when under very heavier load
  remus::server::ServerPorts tcp_ports = remus::server::ServerPorts();
  boost::shared_ptr<remus::Server> server = make_Server( tcp_ports );
  tcp_ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( tcp_ports );

  std::vector< boost::shared_ptr<remus::Worker> > workers;
  workers.push_back( make_Worker( tcp_ports ) );
  workers.push_back( make_Worker( tcp_ports ) );
  workers.push_back( make_Worker( tcp_ports ) );
  workers.push_back( make_Worker( tcp_ports ) );

  //submit 1024 jobs, with a random binary blob. this is what the worker
  //will use as status
  const std::size_t num_jobs = 1024;
  std::vector< remus::proto::Job > jobs;
  for( std::size_t i=0; i < num_jobs; ++i)
    {
    jobs.push_back(submit_Job(client));
    }

  //round-robin 50% of the jobs to different workers. For each job
  //send a nice sized job status message for the client to fetch
  for(std::size_t i = 0; i < (num_jobs/2); ++i)
    {
    remus::worker::Job wjob = workers[i%4]->getJob();
    remus::proto::JobProgress jprogress( wjob.details("blob") );
    remus::proto::JobStatus status( wjob.id(), jprogress );
    workers[i%4]->updateStatus( status );
    }

  client_query_performance(jobs, client);

  return 0;
}
