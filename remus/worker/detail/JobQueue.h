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

//A FIFO queue. each mesh type has its own queue
//where we keep jobs. The uuid for each job
//must be unique.
class JobQueue
{
public:
  JobQueue(zmq::context_t& context,
           const zmq::socketInfo<zmq::proto::inproc>& queue_info);
  ~JobQueue();

  std::string endpoint() const;

  //Removes the first job from the queue
  remus::worker::Job take();

  //return the number of jobs waiting for work
  std::size_t size() const;

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