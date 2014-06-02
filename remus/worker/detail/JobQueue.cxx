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

#include <remus/worker/detail/JobQueue.h>

#include <remus/proto/Response.h>
#include <remus/proto/JobSubmission.h>

//suppress warnings inside boost headers for gcc and clang
#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <boost/thread.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <boost/thread/locks.hpp>

#include <deque>
#include <map>

namespace remus{
namespace worker{
namespace detail{

//-----------------------------------------------------------------------------
class JobQueue::JobQueueImplementation
{
  //needed to talk to the location that is queuing jobs
  zmq::socket_t ServerComm;

  //thread our polling method
  boost::scoped_ptr<boost::thread> PollingThread;

  //used to keep the queue from breaking with threads
  boost::mutex QueueMutex;
  boost::condition_variable QueueChanged;
  std::deque< remus::worker::Job > Queue;

  //need to store our endpoint so we can pass it to the worker
  std::string EndPoint;

  //state to tell when we should stop polling
  bool ContinuePolling;

public:
//-----------------------------------------------------------------------------
JobQueueImplementation(zmq::context_t& context,
                       const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  ServerComm(context,ZMQ_PAIR),
  PollingThread(new boost::thread()),
  QueueMutex(),
  QueueChanged(),
  Queue(),
  EndPoint(queue_info.endpoint()),
  ContinuePolling(true)
{
  //bind to the work_jobs communication channel first
  this->ServerComm.bind( this->EndPoint.c_str() );

  //start up our thread
  boost::scoped_ptr<boost::thread> pollingThread(
      new boost::thread( &JobQueueImplementation::pollForJobs,
                         this) );

  //transfer ownership of the polling thread to our scoped_ptr
  this->PollingThread.swap(pollingThread);
}

//------------------------------------------------------------------------------
~JobQueueImplementation()
{
  //stop the thread
  this->PollingThread->join();
}

//------------------------------------------------------------------------------
const std::string& endpoint() const
{
  return this->EndPoint;
}

//------------------------------------------------------------------------------
void stop()
{
  this->ContinuePolling = false;
}

//------------------------------------------------------------------------------
void pollForJobs()
{
  zmq::pollitem_t item  = { this->ServerComm,  0, ZMQ_POLLIN, 0 };
  while( this->ContinuePolling )
    {
    zmq::poll(&item,1,250);
    if(item.revents & ZMQ_POLLIN)
      {
      remus::proto::Response response(&this->ServerComm);
      switch(response.serviceType())
        {
        case remus::TERMINATE_WORKER:
          this->clearJobs();
          this->stop();
          break;
        case remus::MAKE_MESH:
          this->addItem(response);
          break;
        case remus::TERMINATE_JOB:
          this->terminateJob(response);
        default:
          //ignore other service types as we shouldn't be sent those
          break;
        }
      }
    }
}

void terminateJob(remus::proto::Response& response)
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);
  remus::worker::Job tj = remus::worker::to_Job(response.data());
  for (std::deque<remus::worker::Job>::iterator i = this->Queue.begin();
       i != this->Queue.end(); ++i)
    {
    if(i->id() == tj.id())
      i->updateValidityReason(remus::worker::Job::INVALID);
    }
  this->QueueChanged.notify_all();
}

//------------------------------------------------------------------------------
void clearJobs()
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);
  this->Queue.clear();

  remus::worker::Job j;
  j.updateValidityReason(remus::worker::Job::TERMINATE_WORKER);
  this->Queue.push_back(j);

  this->QueueChanged.notify_all();
}

//------------------------------------------------------------------------------
void addItem(remus::proto::Response& response )
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);
  remus::worker::Job j = remus::worker::to_Job(response.data());
  this->Queue.push_back( j );

  this->QueueChanged.notify_all();
}

//------------------------------------------------------------------------------
remus::worker::Job take()
{
  //remove all jobs from the front that are invalid
  //until we hit a valid job or the terminate worker job
  remus::worker::Job msg;
  while(this->size() > 0 &&
        msg.validityReason() == remus::worker::Job::INVALID)
    {
      {
      boost::unique_lock<boost::mutex> lock(this->QueueMutex);
      msg = this->Queue[0];

      this->Queue.pop_front();
      }
    }
  return msg;
}

//------------------------------------------------------------------------------
remus::worker::Job waitAndTakeJob()
{
  boost::unique_lock<boost::mutex> lock(this->QueueMutex);
  while(this->Queue.size() == 0)
    {
    QueueChanged.wait(lock);
    }
  lock.unlock();
  return take();
}

//------------------------------------------------------------------------------
std::size_t size()
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);
  return this->Queue.size();
}

};

//------------------------------------------------------------------------------
JobQueue::JobQueue(zmq::context_t& context,
                   const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  Implementation( new JobQueueImplementation(context,queue_info) )
{
}

JobQueue::~JobQueue()
{
  this->Implementation->stop();
}

//------------------------------------------------------------------------------
std::string JobQueue::endpoint() const
{
  return this->Implementation->endpoint();
}

//------------------------------------------------------------------------------
remus::worker::Job JobQueue::take()
{
  return this->Implementation->take();
}

//------------------------------------------------------------------------------
remus::worker::Job JobQueue::waitAndTakeJob()
{
  return this->Implementation->waitAndTakeJob();
}

//------------------------------------------------------------------------------
std::size_t JobQueue::size() const
{
  return this->Implementation->size();
}


}
}
} //namespace remus::worker::detail
