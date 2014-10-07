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
  static std::size_t small_size = 1024;
  static std::size_t large_size = 16777216;

  static std::size_t ascii_data_size = small_size;
  static std::size_t binary_data_size = small_size;

//------------------------------------------------------------------------------
struct WorkerController
{

  boost::scoped_ptr< boost::thread > Monitor;
  boost::shared_ptr< remus::Worker > Worker;
  boost::shared_ptr<bool> ContinueTakingJobs;

  //----------------------------------------------------------------------------
  WorkerController( boost::shared_ptr< remus::Worker > w):
    Monitor(),
    Worker(w),
    ContinueTakingJobs( new bool(false) )
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
  *this->ContinueTakingJobs = true;
  boost::scoped_ptr<boost::thread> sthread(
        new  boost::thread(&WorkerController::start_impl,
                           this,
                           this->ContinueTakingJobs) );
  this->Monitor.swap(sthread);
  }

  //----------------------------------------------------------------------------
  void stop()
  {
  *this->ContinueTakingJobs = false;
  this->Monitor->join();
  }

private:
  //----------------------------------------------------------------------------
  void start_impl(boost::shared_ptr<bool> continueTalking)
  {
  while(*continueTalking == true)
    {
    //don't block asking for a job, instead do a slow poll. First we ask
    //for a job, than we wait for the job to arrive, and once we have it
    //we take it
    this->Worker->askForJobs();
    while ( *continueTalking && this->Worker->pendingJobCount() == 0)
      {//ContinueTakingJobs allows us to shutdown while polling for a job
      remus::common::SleepForMillisec( 25 );
      boost::this_thread::yield();
      }
    if(*continueTalking == false) {return ;}

    //now verify the job doesn't tell us to quit
    remus::worker::Job jd = this->Worker->takePendingJob( );
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
      if( progress%20==0 )
        {
        jprogress.setValue( progress );
        jprogress.setMessage( "Example Message With Random Content" );
        status.updateProgress( jprogress );
        this->Worker->updateStatus( status );
        std::cout << "Worker sending progress " << progress << "%" << std::endl;
        remus::common::SleepForMillisec( 350 );
        }
      }

    //respond by modifying the job content
    std::string binary_output(content.data(), content.dataSize() );
    binary_output += remus::testing::AsciiStringGenerator( ascii_data_size );

    remus::proto::JobResult results =
          remus::proto::make_JobResult( jd.id(), binary_output );

    this->Worker->returnResult( results );
    }
  std::cout << "worker told to stop accepting jobs from server" << std::endl;
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
bool verify_jobs(boost::shared_ptr<remus::Client> client,
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
  unsigned int num_valid_finished_jobs = 0;
  while(jobs.size() > 0)
    {
    for(std::size_t i=0; i < jobs.size(); ++i)
      {
      JobStatus status = client->jobStatus( jobs[i] );
      if ( !status.good() )
        {
        std::cout << "job finished with status: "
                  <<  remus::to_string( status.status() ) << std::endl;
        if( status.finished() )
          {
          //mark the job as finished properly if the result are larger
          //than the job we submitted.
          JobResult r = client->retrieveResults( jobs[i] );
          if( r.dataSize() >  binary_data_size  )
            { num_valid_finished_jobs++; }
          }
        //remove the job from the list to poll
        jobs.erase(jobs.begin()+i);
        }
      }
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
  //construct a simple worker and client
  boost::shared_ptr<remus::Server> server = make_Server( remus::server::ServerPorts() );
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  boost::shared_ptr<remus::Client> client = make_Client( ports );

  //if no parameters just run with a single worker and single job
  int num_workers = 1;
  int num_jobs = 1;
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

  std::vector < WorkerController* > processors;
  for (int i=0; i < num_workers; ++i)
    {
    WorkerController* wc = new WorkerController( make_Worker( ports ) );
    processors.push_back( wc );
    }

  const bool valid = verify_jobs(client, processors, num_jobs);

  //delete workers before validating the results
  for (std::size_t i=0; i < processors.size(); ++i)
    {
    WorkerController* wc = processors[i];
    wc->stop();
    delete wc;
    processors[i] = NULL;
    }
  processors.clear();

  //now verify that all the jobs finished properly
  return ( valid == true ) ? 0 : 1;
}
