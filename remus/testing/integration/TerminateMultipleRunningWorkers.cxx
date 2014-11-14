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
  static std::size_t ascii_data_size = 1024;
  static std::size_t binary_data_size = 1024;

//------------------------------------------------------------------------------
struct WorkerController
{

  boost::scoped_ptr< boost::thread > Monitor;
  boost::shared_ptr< remus::Worker > Worker;

  //----------------------------------------------------------------------------
  WorkerController( boost::shared_ptr< remus::Worker > w):
    Monitor(),
    Worker(w)
  {
  }

  //----------------------------------------------------------------------------
  ~WorkerController()
  {
  this->stop();
  }

  //----------------------------------------------------------------------------
  void start()
  {
  boost::scoped_ptr<boost::thread> sthread(
        new  boost::thread(&WorkerController::start_impl, this) );
  this->Monitor.swap(sthread);
  }

  //----------------------------------------------------------------------------
  void stop()
  {
  this->Monitor->join();
  }

private:
  //----------------------------------------------------------------------------
  void start_impl()
  {
  //keep going while we have jobs to do
   while( true )
    {
    boost::this_thread::yield();

    //now verify the job doesn't tell us to quit
    remus::worker::Job jd = this->Worker->getJob( );
    switch(jd.validityReason())
      {
        case remus::worker::Job::INVALID:
          std::cout << "worker given invalid job" << std::endl;
          return;
        case remus::worker::Job::TERMINATE_WORKER:
          std::cout << "worker told to terminate" << std::endl;
          return;
        case remus::worker::Job::VALID_JOB:
        default:
          break;
      }

    const remus::proto::JobContent& content =
                                    jd.submission().find( "binary" )->second;

    remus::proto::JobProgress jprogress;
    remus::proto::JobStatus status( jd.id(), remus::IN_PROGRESS );
    for( int progress=1; progress <= 100; ++progress )
      {
      jprogress.setValue( progress );
      jprogress.setMessage( "Example Message With Random Content" );
      for( int i=1; i < 100; ++i )
        {
        status.updateProgress( jprogress );
        this->Worker->updateStatus( status );
        }
      }

    //respond by modifying the job content
    std::string binary_output(content.data(), content.dataSize() );
    binary_output += remus::testing::AsciiStringGenerator( ascii_data_size );

    remus::proto::JobResult results =
          remus::proto::make_JobResult( jd.id(), binary_output );

    this->Worker->returnResult( results );
    }
  }
};

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
remus::proto::Job submit_Job(boost::shared_ptr<remus::Client> client,
                             remus::proto::JobContent binary_content)
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  remus::common::MeshIOType io_type = remus::common::make_MeshIOType(Mesh2D(),Mesh3D());
  JobRequirements reqs = make_JobRequirements(io_type, "SimpleWorker", "");

  JobSubmission sub(reqs);
  sub["binary"]=binary_content;
  remus::proto::Job job = client->submitJob(sub);
  REMUS_ASSERT(job.valid())
  return job;
}

//------------------------------------------------------------------------------
void terminate_blocking_workers(boost::shared_ptr<remus::Client> client,
                                boost::shared_ptr<remus::Server> server,
                                std::vector< WorkerController* >& processors,
                                std::size_t num_jobs_to_submit)
{
  using namespace remus::proto;

  //first task is to start all the worker controllers
  std::cout << "starting up workers" << std::endl;
  for( std::size_t i=0; i < processors.size(); ++i)
    { processors[i]->start(); }
  remus::common::SleepForMillisec( 125 ); //let workers connect to server

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
    for(std::size_t i=0; i < jobs.size(); ++i)
      {
      JobStatus status = client->jobStatus( jobs[i] );
      //once the first job is finished we kill all workers
      if( status.finished() )
        {
        ++num_finished_jobs;
        }
      if(num_finished_jobs == 3)
        {
        std::cout << "stopBrokering" << std::endl;
        jobs.clear();
        server->stopBrokering();
        }
      }
    }
}

}

//
int TerminateMultipleRunningWorkers(int, char**)
{
  //construct a simple worker and client
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );

  //if no parameters just run with a single worker and single job
  std::size_t num_workers = 8;
  std::size_t num_jobs = 100;
  std::string data_size_flag = "small";

  std::cout << "verifying "
            << num_workers << " workers "
            << num_jobs << " jobs" << std::endl;

  std::vector < WorkerController* > processors;
  for (std::size_t i=0; i < num_workers; ++i)
    {
    WorkerController* wc  = new WorkerController( make_Worker( ports ) );
    processors.push_back( wc );
    }

  terminate_blocking_workers(client, server, processors, num_jobs);

  //keep looking at the workers to see when they stop running
  for (std::size_t i=0; i < processors.size(); ++i)
    {
    WorkerController* wc = processors[i];
    wc->stop();
    }


  processors.clear();
  return 0;
}
