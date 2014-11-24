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
#ifndef remus_testing_integration_detail_Workers_h
#define remus_testing_integration_detail_Workers_h

#include <remus/worker/Worker.h>
#include <remus/common/SleepFor.h>
#include <remus/testing/Testing.h>

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <iostream>

namespace
{
int randomInt(int min_v, int max_v)
{
  typedef boost::uniform_int<> NumberDistribution;
  typedef boost::mt19937 RandomNumberGenerator;
  typedef boost::variate_generator<RandomNumberGenerator&,
                                     NumberDistribution> Generator;

  static NumberDistribution distribution(min_v, max_v);
  static RandomNumberGenerator generator;
  static Generator numberGenerator(generator, distribution);
  return numberGenerator();
}
}
namespace remus {
namespace testing {
namespace integration {
namespace detail {

/*
  SingleShotWorker is a threaded controller that runs a worker only once
  and generates a random amount of message load to the server.
*/
//------------------------------------------------------------------------------
class SingleShotWorker : public remus::worker::Worker
{
public:

  //----------------------------------------------------------------------------
  SingleShotWorker(const remus::proto::JobRequirements& requirements,
         const remus::worker::ServerConnection& conn):
  remus::worker::Worker(requirements,conn),
  numCompletedJobs(0)
  {
    this->start_impl();
  }

  //----------------------------------------------------------------------------
  std::size_t numberOfCompletedJobs() const { return this->numCompletedJobs; }

private:
  //----------------------------------------------------------------------------
  void start_impl()
  {
  typedef boost::posix_time::ptime ptime;
  typedef boost::posix_time::time_duration time_duration;

  //this is used to generate artificial server messaging load.
  const std::size_t numberOfMessagesToSend = randomInt(250, 2000);
  std::cout << "Worker is going to send: " << 20 * numberOfMessagesToSend << " messages" << std::endl;

  //We rely on the server telling us to terminate for us to return
  //from get job. The job in that case should be a TERMINATE_WORKER
  //job.

//  const ptime askForJobStart = boost::posix_time::microsec_clock::local_time();
  remus::worker::Job jd = this->getJob();
//  const ptime askForJobEnd = boost::posix_time::microsec_clock::local_time();

//  std::cout << "Time to get a job is: " << askForJobEnd - askForJobStart << std::endl;
  switch(jd.validityReason())
    {
    case remus::worker::Job::TERMINATE_WORKER:
      std::cout << "worker is terminating" << std::endl;
      return;
    case remus::worker::Job::VALID_JOB:
    default:
      break;
    }

  remus::proto::JobProgress jprogress;
  remus::proto::JobStatus status( jd.id(), remus::IN_PROGRESS );
  for( std::size_t progress=1; progress <= 100; progress+=5)
    {

    jprogress.setValue( progress );

    //send a random ascii status message back to the client
    jprogress.setMessage( remus::testing::AsciiStringGenerator(1024) );

    status.updateProgress( jprogress );

    for( std::size_t i=0; i < numberOfMessagesToSend; ++i)
      { //send multiple times to simulate load
      this->updateStatus( status );
      }
    }

  //respond by generate some random data the job content
  const std::size_t binaryDataSize = 1024 * numberOfMessagesToSend;
  std::string binary_output = remus::testing::BinaryDataGenerator( binaryDataSize );

  remus::proto::JobResult results =
        remus::proto::make_JobResult( jd.id(), binary_output );

  ++this->numCompletedJobs;
  this->returnResult( results );
  }

  std::size_t numCompletedJobs;
};


/*
  InfiniteWorkerController is a threaded controller that runs a worker
  and generates a random amount of message load to the server

  todo: remove the loop from start_impl and instead have the factory
        call start each time
*/
//------------------------------------------------------------------------------
struct InfiniteWorkerController
{

  boost::scoped_ptr< boost::thread > Monitor;
  boost::shared_ptr< remus::Worker > Worker;
  std::size_t numCompletedJobs;

  //----------------------------------------------------------------------------
  InfiniteWorkerController( boost::shared_ptr< remus::Worker > w):
    Monitor(),
    Worker(w),
    numCompletedJobs(0)
  {
  }

  //----------------------------------------------------------------------------
  ~InfiniteWorkerController()
  {
  this->stop();
  }

  //----------------------------------------------------------------------------
  void start()
  {
  boost::scoped_ptr<boost::thread> sthread(
        new  boost::thread(&InfiniteWorkerController::start_impl, this) );
  this->Monitor.swap(sthread);
  }

  //----------------------------------------------------------------------------
  std::size_t numberOfCompletedJobs() const { return this->numCompletedJobs; }

  //----------------------------------------------------------------------------
  void stop()
  {
  this->Monitor->join();
  }

private:
  //----------------------------------------------------------------------------
  void start_impl()
  {
  std::srand(static_cast<unsigned int>(std::time(0)));

  while(true)
    {
    //this is used to generate artificial server messaging load.
    const std::size_t numberOfMessagesToSend = 10 + std::rand() % (100);

    //We rely on the server telling us to terminate for us to return
    //from get job. The job in that case should be a TERMINATE_WORKER
    //job.

    remus::worker::Job jd = this->Worker->getJob();
    switch(jd.validityReason())
      {
      case remus::worker::Job::TERMINATE_WORKER:
        std::cout << "worker is terminating" << std::endl;
        return;
      case remus::worker::Job::VALID_JOB:
      default:
        break;
      }

    const remus::proto::JobContent& content =
                                    jd.submission().find( "binary" )->second;

    const remus::proto::JobContent& ascii_content_size =
                                    jd.submission().find( "ascii_data_size" )->second;

    std::size_t ascii_data_size = 0;
    std::stringstream ascii_buffer;
    ascii_buffer << std::string(ascii_content_size.data());
    ascii_buffer >> ascii_data_size;


    remus::proto::JobProgress jprogress;
    remus::proto::JobStatus status( jd.id(), remus::IN_PROGRESS );
    for( std::size_t  progress=1; progress <= 100; progress+=5)
      {

      jprogress.setValue( progress );

      //send a random ascii status message back to the client
      jprogress.setMessage( remus::testing::AsciiStringGenerator(1024) );

      status.updateProgress( jprogress );

      for( std::size_t i=0; i < numberOfMessagesToSend; ++i)
        { //send multiple times to simulate load
        this->Worker->updateStatus( status );
        }
      }

    //respond by modifying the job content
    std::string binary_output(content.data(), content.dataSize() );
    binary_output += remus::testing::AsciiStringGenerator( ascii_data_size );

    remus::proto::JobResult results =
          remus::proto::make_JobResult( jd.id(), binary_output );

    ++this->numCompletedJobs;
    this->Worker->returnResult( results );
    }
  }
};

}
}
}
}
#endif //remus_testing_integration_detail_Workers_h
