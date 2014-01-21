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

#include <remus/common/Response.h>
#include <remus/worker/Job.h>

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

#include <queue>


namespace remus{
namespace worker{
namespace detail{

//-----------------------------------------------------------------------------
class JobQueue::JobQueueImplementation
{
  //needed to talk to the location that is queueing jobs
  zmq::socket_t ServerComm;

  //thread our polling method
  boost::scoped_ptr<boost::thread> PollingThread;

  //used to keep the queue from breaking with threads
  boost::mutex QueueMutex;
  std::queue<std::string> Queue;

  //state to tell when we should stop polling
  bool ContinuePolling;

public:
  //need to store our endpoint so we can pass it to the worker
  std::string EndPoint;

JobQueueImplementation(zmq::context_t& context):
  ServerComm(context,ZMQ_PAIR),
  PollingThread(NULL),
  QueueMutex(),
  Queue(),
  EndPoint(),
  ContinuePolling(true)
{
  //bind to the work_jobs communication channel first
  zmq::socketInfo<zmq::proto::inproc> internalCommInfo =
    zmq::bindToAddress<zmq::proto::inproc>(this->ServerComm,"worker_jobs");
  this->EndPoint = internalCommInfo.endpoint();

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
    zmq::poll(&item,1,remus::HEARTBEAT_INTERVAL);
    if(item.revents & ZMQ_POLLIN)
      {
      remus::common::Response response(this->ServerComm);
      this->addItem(response);
      }
    }
}

//------------------------------------------------------------------------------
void addItem(remus::common::Response& response )
{
  this->QueueMutex.lock();
  this->Queue.push(response.dataAs<std::string>());
  this->QueueMutex.unlock();
}

//------------------------------------------------------------------------------
remus::worker::Job take()
{
  if(this->size() > 0)
    {
    this->QueueMutex.lock();
    std::string msg = this->Queue.front();
    this->Queue.pop();
    this->QueueMutex.unlock();
    return remus::worker::to_Job(msg);
    }
  return remus::worker::Job();
}

//------------------------------------------------------------------------------
std::size_t size()
{
  this->QueueMutex.lock();
  const std::size_t s = this->Queue.size();
  this->QueueMutex.unlock();
  return s;
}

};

//------------------------------------------------------------------------------
JobQueue::JobQueue(zmq::context_t& context):
  Implementation( new JobQueueImplementation(context) )
{
}

JobQueue::~JobQueue()
{
  this->Implementation->stop();
  delete this->Implementation;
  this->Implementation = NULL;
}

//------------------------------------------------------------------------------
std::string JobQueue::endpoint() const
{
  return this->Implementation->EndPoint;
}

//------------------------------------------------------------------------------
remus::worker::Job JobQueue::take()
{
  return this->Implementation->take();
}

//------------------------------------------------------------------------------
std::size_t JobQueue::size() const
{
  return this->Implementation->size();
}


}
}
} //namespace remus::worker::detail
