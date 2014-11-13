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
//=============================================================================

#ifndef remus_worker_detail_JobQueue_h
#define remus_worker_detail_JobQueue_h

#include <remus/proto/zmqHelper.h>
#include <remus/worker/Job.h>

#include <boost/scoped_ptr.hpp>

namespace remus{
namespace worker{
namespace detail{

//A Simple JobQueue that holds onto a collection of jobs from the server.
//If the TermianteWorker job is sent to the job queue, we will clear the
//entire queue and only have a TerminateJob on the queue.
//
//Once a JobQueue is sent a TerminateWorker, it will not accept any new jobs
//and will refuse to start back up looking for jobs
class JobQueue
{
public:
  JobQueue(zmq::context_t& context,
           const zmq::socketInfo<zmq::proto::inproc>& queue_info);
  ~JobQueue();

  std::string endpoint() const;

  //Returns true if the job is part of the queue and job status
  //has been marked as terminate. This is allows people to peek at the queue
  //and check jobs without taking them off the queue
  bool isATerminatedJob( const remus::worker::Job& job) const;

  //Removes the first job from the queue, If no job
  //in the queue will return an invalid job
  remus::worker::Job take();

  //Removes the first job from the queue, If no
  //job is present, it waits for a job to enter the queue
  remus::worker::Job waitAndTakeJob();

  //return the number of jobs waiting for work
  std::size_t size() const;

  //has finished setting up and is ready for jobs
  bool isReady() const;

  //has job queue been told to shutdown and terminate this is only true
  //if the job queue has been started and than has been stopped. If the
  //job queue is still launching this will return false
  bool isShutdown() const;

private:
  class JobQueueImplementation;
  boost::scoped_ptr<JobQueueImplementation> Implementation;

  //make copying not possible
  JobQueue (const JobQueue&);
  void operator = (const JobQueue&);
};

}
}
}

#endif