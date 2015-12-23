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
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <deque>
#include <set>

namespace
{
struct JobIdMatches
  {
    JobIdMatches(boost::uuids::uuid id):
    UUID(id) {}

    bool operator()(const remus::worker::Job& job) const
      { return job.id() == UUID; }

    boost::uuids::uuid UUID;
  };
}

namespace remus{
namespace worker{
namespace detail{

//-----------------------------------------------------------------------------
class JobQueue::JobQueueImplementation
{
  //thread our polling method
  boost::scoped_ptr<boost::thread> PollingThread;

  //used to keep the queue from breaking with threads
  boost::mutex QueueMutex;
  boost::condition_variable QueueChanged;
  std::deque< remus::worker::Job > Queue;

  //a set of jobs that the JobQueue has been told should be terminated
  std::set< boost::uuids::uuid > TerminatedJobs;

  //need to store our endpoint so we can pass it to the worker
  std::string EndPoint;

  //state to tell when we should stop polling
  bool ContinuePolling;

  //states that we have finished setting up and are ready to accept messages
  bool PollingStarted;

  //states that we have been told to stop polling and should be ready
  //to be destroyed. Is not the same as NOT ContinuePolling and PollingStarted.
  //as that logic returns false while the polling thread is being constructed
  bool PollingFinished;

public:
//-----------------------------------------------------------------------------
JobQueueImplementation(zmq::context_t& context,
                       const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  PollingThread(new boost::thread()),
  QueueMutex(),
  QueueChanged(),
  Queue(),
  TerminatedJobs(),
  EndPoint(),
  ContinuePolling(true),
  PollingStarted(false),
  PollingFinished(false)
{
  //start up our thread
  boost::scoped_ptr<boost::thread> pollingThread(
      new boost::thread( &JobQueueImplementation::pollForJobs,
                         this,
                         &context,
                         queue_info) );

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
  this->PollingStarted = false;
  this->PollingFinished = true;
}

//------------------------------------------------------------------------------
void pollForJobs(zmq::context_t* context,
                 zmq::socketInfo<zmq::proto::inproc> queue_info)
{
  //we pass the context by pointer since it can't be copied.
  //we pass the queue_info by value since it is light weight

  //since this is threaded, this we need to make sure that zmq_socket and
  //zmq_close will execute from inside the thread address space so that
  //when the thread is joined everything cleans up in the correct order,
  //otherwise we can get segment-faults when trying to use multiple workers in
  //the same process.
  zmq::socket_t serverComm(*context,ZMQ_PAIR);

  //bind to the work_jobs communication channel first
  this->EndPoint = zmq::bindToAddress(serverComm, queue_info).endpoint();

  //now that we have finished binding we are ready to accept jobs
  this->PollingStarted = true;

  zmq::pollitem_t item  = { serverComm,  0, ZMQ_POLLIN, 0 };
  while( this->ContinuePolling )
    {
    zmq::poll_safely(&item,1,250);
    if(item.revents & ZMQ_POLLIN)
      {
      remus::proto::Response response =
          remus::proto::receive_Response(&serverComm);
      if(!response.isValid())
        { //ignore this response if it isn't valid
        continue;
        }

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

//------------------------------------------------------------------------------
void terminateJob(remus::proto::Response& response)
{
  boost::lock_guard<boost::mutex> lock(this->QueueMutex);

  const std::string data(response.data(), response.dataSize());
  remus::worker::Job tj = remus::worker::to_Job(data);

  //first thing is we add the job id to the list of terminated job ids
  this->TerminatedJobs.insert( tj.id() );

  //next we go through the deque and remove any job with that id

  typedef std::deque< remus::worker::Job >::iterator iter;
  JobIdMatches pred( tj.id() );

  iter new_end = std::remove_if(this->Queue.begin(),
                                this->Queue.end(),
                                pred);
  this->Queue.erase(new_end,this->Queue.end());

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

  //required to use the char*, len constructor as response's data can
  //be binary data with lots of null terminators.
  const std::string data(response.data(), response.dataSize());
  remus::worker::Job j = remus::worker::to_Job(data);
  this->Queue.push_back( j );

  this->QueueChanged.notify_all();
}

//------------------------------------------------------------------------------
bool isATerminatedJob(const remus::worker::Job& job) const
{
  return this->TerminatedJobs.count( job.id() ) == 1;
}

//------------------------------------------------------------------------------
remus::worker::Job take()
{
  //the only jobs on the queue should be valid jobs or kill the worker
  remus::worker::Job job;
  if(this->size() > 0)
    {
    boost::unique_lock<boost::mutex> lock(this->QueueMutex);
    job = this->Queue[0];
    this->Queue.pop_front();

    //notify everyone that a job was taken from the queue
    this->QueueChanged.notify_all();
    }
  return job;
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

//------------------------------------------------------------------------------
bool isReady() const
{
  return this->PollingStarted && this->ContinuePolling;
}

//------------------------------------------------------------------------------
bool isShutdown() const
{
  return this->PollingFinished;
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
bool JobQueue::isATerminatedJob(const remus::worker::Job& job) const
{
  return this->Implementation->isATerminatedJob(job);
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

//------------------------------------------------------------------------------
bool JobQueue::isReady() const
{
  return this->Implementation->isReady();
}

//------------------------------------------------------------------------------
bool JobQueue::isShutdown() const
{
  return this->Implementation->isShutdown();
}

}
}
} //namespace remus::worker::detail
