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


#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/date_time/posix_time/posix_time.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

namespace
{

static std::size_t blob_size = 512;
static std::size_t num_messages = 100000;

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
void worker_transfer_performance( boost::shared_ptr< remus::Worker > worker)
{
  typedef boost::posix_time::ptime ptime;
  typedef boost::posix_time::time_duration time_duration;

  remus::worker::Job wjob = worker->getJob();

  //verify that the wjob is valid
  REMUS_ASSERT( (wjob.valid()) )

  //extract and copy the blob data
  std::string blob = wjob.details("blob");

  const ptime startTime = boost::posix_time::microsec_clock::local_time();
  remus::proto::JobProgress jprogress(blob);
  for( std::size_t i=0; i < num_messages; ++i)
    {
    //send a new status message
    remus::proto::JobStatus status( wjob.id(), jprogress );
    worker->updateStatus( status );
    }

  //to make sure all the status have actually been sent, we need to send
  // a job result, and try to shutdown the worker
  worker->returnResult( remus::proto::JobResult(wjob.id()) );
  worker.reset(); //shutdown the worker

  const ptime endTime = boost::posix_time::microsec_clock::local_time();
  const time_duration dur = endTime - startTime;

  //sizeof char in c++ == 1, so length of string is size in bytes
  remus::proto::JobStatus example_status( wjob.id(), jprogress );
  const boost::int64_t status_byte_size =
    static_cast<boost::int64_t>( remus::proto::to_string(example_status).size() );
  const boost::int64_t total_bytes_sent = num_messages * status_byte_size;

  std::cout << "Single message size " <<  status_byte_size << std::endl;
  std::cout << "Total bytes sent " <<  total_bytes_sent << " across  " << num_messages << " messages." << std::endl;

  const boost::int64_t messages_per_msec = (num_messages/dur.total_milliseconds());
  std::cout << "Messages per sec " << (messages_per_msec * 1000)  << std::endl;

  const boost::int64_t bytes_per_sec = 1000 * (total_bytes_sent / dur.total_milliseconds() );
  const boost::int64_t mb_per_sec = bytes_per_sec / 1048576;
  std::cout << "Bandwidth per sec " << mb_per_sec << " (MB/sec) "<< std::endl;


  //we should never be below this rage. If you add code and this line
  //starts returning false, you need to improve the performance of your feature
#if defined(__APPLE__)
  REMUS_ASSERT( (mb_per_sec >= 10))
#elif defined(_WIN32)
  REMUS_ASSERT( (mb_per_sec >= 10))
#else
  //unix just does better with loopback
  REMUS_ASSERT( (mb_per_sec >= 14))
#endif

  //Okay, we have this test here, so in case we improve performance, that means
  //we can update the baseline performance. So if you start seeing this line
  //starting to fail, you should increase both our ceiling and floor for performance
#if defined(__APPLE__)
  REMUS_ASSERT( (mb_per_sec <= 25))
#elif defined(_WIN32)
  REMUS_ASSERT( (mb_per_sec <= 25))
#else
  //unix just does better with loopback
  REMUS_ASSERT( (mb_per_sec <= 65))
#endif
}



}

int main(int argc, char* argv[])
{
  (void) argc;
  (void) argv;

  //construct a simple worker and client
  remus::server::ServerPorts tcp_ports = remus::server::ServerPorts();
  boost::shared_ptr<remus::Server> server = make_Server( tcp_ports );
  tcp_ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( tcp_ports );
  boost::shared_ptr<remus::Worker> worker = make_Worker( tcp_ports );

  //submit a job, with a random binary blob. this is what the worker
  //will use as status
  submit_Job(client);

  //INVALIDATES the worker shared_ptr!!!!!
  worker_transfer_performance(worker);


  return 0;
}
