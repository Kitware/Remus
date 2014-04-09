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

#include <deque>
#include <map>


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
  boost::condition_variable QueueChanged;

  std::deque< remus::worker::Job > Queue;

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
    zmq::poll(&item,1,250);
    if(item.revents & ZMQ_POLLIN)
      {
      remus::common::Response response(this->ServerComm);
      switch(response.serviceType())
        {
        case remus::TERMINATE_WORKER:
          this->stop();
          this->clearJobs();
          break;
        case remus::MAKE_MESH:
          this->addItem(response);
          break;
        case remus::TERMINATE_JOB:
          this->terminateJob(response);
        }
      }
    }
}

void terminateJob(remus::common::Response& response)
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);
  remus::worker::Job tj = remus::worker::to_Job(response.dataAs<std::string>());
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
  this->Queue.push_back(remus::worker::Job(remus::worker::Job::TERMINATE_WORKER));
  this->QueueChanged.notify_all();
}

//------------------------------------------------------------------------------
void addItem(remus::common::Response& response )
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);
  this->Queue.push_back( remus::worker::to_Job(response.dataAs<std::string>()) );
  this->QueueChanged.notify_all();
}

//------------------------------------------------------------------------------
remus::worker::Job take()
{
  while(this->size() > 0)
    {
    remus::worker::Job msg;
      {
      boost::lock_guard<boost::mutex> lock(this->QueueMutex);
      msg = this->Queue.front();
      this->Queue.pop_front();
      }
    switch(msg.validityReason())
      {
      case remus::worker::Job::INVALID:
        continue;
      case remus::worker::Job::TERMINATE_WORKER:
      case remus::worker::Job::VALID_JOB:
        return msg;
      }
    }
  return remus::worker::Job();
}

//------------------------------------------------------------------------------
remus::worker::Job waitAndTakeJob()
{
  boost::unique_lock<boost::mutex> lock(this->QueueMutex);
    while(this->size() == 0)
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
