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

#ifndef __remus_worker_detail_JobQueue_h
#define __remus_worker_detail_JobQueue_h

#include <remus/worker/Job.h>
#include <remus/common/zmqHelper.h>

namespace remus{
namespace worker{
namespace detail{

//A FIFO queue. each mesh type has its own queue
//where we keep jobs. The uuid for each job
//must be unique.

class JobQueue
{
public:
  JobQueue(zmq::context_t& context);
  ~JobQueue();

  std::string endpoint() const;

  //Removes the first job from the queue, If no job
  //in the queue will return an invalid job
  remus::worker::Job take();

  //Removes the first job from the queue, If no
  //job is present, it waits for a job to enter the queue
  remus::worker::Job waitAndTakeJob();

  //return the number of jobs waiting for work
  std::size_t size() const;

private:

  class JobQueueImplementation;
  JobQueueImplementation *Implementation;

  //make copying not possible
  JobQueue (const JobQueue&);
  void operator = (const JobQueue&);
};

}
}
}

#endif