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

#include <remus/worker/detail/MessageRouter.h>

#include <remus/proto/Message.h>
#include <remus/proto/Response.h>
#include <remus/proto/zmqHelper.h>

#include <remus/common/PollingMonitor.h>
#include <remus/worker/Job.h>

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/thread.hpp>
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif

#include <boost/thread/locks.hpp>
#include <boost/uuid/uuid.hpp>

namespace remus{
namespace worker{
namespace detail{

//-----------------------------------------------------------------------------
class MessageRouter::MessageRouterImplementation
{
  std::string WorkerEndpoint;
  std::string QueueEndpoint;

  //thread our polling method
  mutable boost::mutex ThreadMutex;
  boost::condition_variable ThreadStatusChanged;

  boost::scoped_ptr<boost::thread> PollingThread;
  //state to tell when we should stop polling
  bool ContinueTalking;
public:
//-----------------------------------------------------------------------------
MessageRouterImplementation(
                      const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                      const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  WorkerEndpoint(worker_info.endpoint()),
  QueueEndpoint(queue_info.endpoint()),
  ThreadMutex(),
  ThreadStatusChanged(),
  PollingThread(new boost::thread()),
  ContinueTalking(false)
{
  //we don't connect the sockets until the polling thread starts up
}

//-----------------------------------------------------------------------------
~MessageRouterImplementation()
{
  if(this->PollingThread)
    {
    this->setIsTalking(false);
    this->PollingThread->join();
    }
}

//-----------------------------------------------------------------------------
bool isTalking() const
{
  boost::lock_guard<boost::mutex> lock(ThreadMutex);
  return this->ContinueTalking;
}

//------------------------------------------------------------------------------
bool startTalking(const remus::worker::ServerConnection& server_info,
                  zmq::context_t& internal_inproc_context)
{
  bool launchThread = false;
  bool threadIsRunning = false;
    //lock the construction of thread as a critical section
    {
    boost::unique_lock<boost::mutex> lock(ThreadMutex);

    //if a thread's id is equal to the default thread id, it means that the
    //thread was never given anything to run, and is actually empty
    threadIsRunning = this->PollingThread->get_id() != boost::thread::id();
    launchThread = !this->ContinueTalking && !threadIsRunning;
    if(launchThread)
      {
      boost::scoped_ptr<boost::thread> pollthread(
        new boost::thread( &MessageRouterImplementation::poll, this,
                            server_info,
                            &internal_inproc_context) );
      this->PollingThread.swap(pollthread);
      }
    }

  //do this outside the previous critical section so that we properly
  //tell other threads that the thread has been launched. We don't
  //want to cause a recursive lock in the same thread to happen
  this->waitForThreadToStart();

  //we can only launch the thread once, and once it has been terminated
  //the entire message router is invalid to start up again.
  return this->isTalking();
}

private:

//----------------------------------------------------------------------------
void setIsTalking(bool t)
{
    {
    boost::unique_lock<boost::mutex> lock(ThreadMutex);
    this->ContinueTalking = t;
    }
  this->ThreadStatusChanged.notify_all();
}

//----------------------------------------------------------------------------
void waitForThreadToStart()
{
  boost::unique_lock<boost::mutex> lock(ThreadMutex);
  while(!this->ContinueTalking)
    {
    ThreadStatusChanged.wait(lock);
    }
}

//------------------------------------------------------------------------------
void poll(remus::worker::ServerConnection server_info,
          zmq::context_t* internal_inproc_context)
{
  //we pass the context by pointer since it can't be copied.
  //we pass the ServerConnection by value since it is light weight

  zmq::socket_t serverComm(*(server_info.context()),ZMQ_DEALER);
  zmq::connectToAddress(serverComm, server_info.endpoint());

  zmq::socket_t queueComm(*internal_inproc_context,ZMQ_PAIR);
  zmq::connectToAddress(queueComm,  this->QueueEndpoint);

  zmq::socket_t workerComm(*internal_inproc_context,ZMQ_PAIR);
  zmq::connectToAddress(workerComm, this->WorkerEndpoint);

  zmq::pollitem_t items[2]  = {
                                { workerComm,  0, ZMQ_POLLIN, 0 },
                                { serverComm,  0, ZMQ_POLLIN, 0 }
                              };

  //we specify a new polling rate so that the polling monitor is more responsive
  //plus allows testing of the monitor to be quicker. In the future we could
  //expose this option in the MessageRouter class.
  remus::common::PollingMonitor monitor(boost::int64_t(250),
                                        boost::int64_t(60*1000) );

  //We need to notify the Thread management that polling is about to start.
  //This allows the calling thread to resume, as it has been waiting for this
  //notification, and will also allow threads that have been holding on
  //waitForThreadToStart to resume
  this->setIsTalking(true);
  while( this->isTalking() )
    {
    bool sentToServer=false;
    zmq::poll(&items[0],2,monitor.current());
    monitor.pollOccurred();

    if(items[0].revents & ZMQ_POLLIN)
      {
      sentToServer = true;

      remus::proto::Message message = remus::proto::receive_Message(&workerComm);

      //special case is that TERMINATE_WORKER means we stop looping
      if(message.serviceType()==remus::TERMINATE_WORKER)
        {
        //send the terminate worker message to the job queue too
        remus::worker::Job terminateJob;
        remus::proto::Response termJob( remus::TERMINATE_WORKER,
                                        remus::worker::to_string(terminateJob));
        termJob.sendNonBlocking(&queueComm, (zmq::SocketIdentity()) );

        this->setIsTalking(false);
        }
      else
        {
        //just pass the message on to the server
        remus::proto::forward_Message(message,&serverComm);
        }
      }
    if(items[1].revents & ZMQ_POLLIN)
      {
      remus::proto::Response response(&serverComm);
      const bool goodToForward = response.isFullyFormed() &&
                                 response.isValidService();
      if(goodToForward)
        {
        switch(response.serviceType())
          {
          case remus::TERMINATE_WORKER:
            response.sendNonBlocking(&queueComm, (zmq::SocketIdentity()));
            this->setIsTalking(false);
            break;
          case remus::TERMINATE_JOB:
            //send the terminate to the job queue since it holds the jobs
          case remus::MAKE_MESH:
            //send the job to the queue so that somebody can take it later
            response.sendNonBlocking(&queueComm, (zmq::SocketIdentity()));
            break;
          default:
            // do nothing if it isn't terminate_job, terminate_worker
            // or make_mesh.
            break;
          }
        }
      }
    if(!sentToServer)
      {
      //send the server how soon in seconds we will send our next heartbeat
      //message. This way we are telling the server itself when it should
      //expect a message, rather than it guessing.
      const boost::int64_t polldur = monitor.hasAbnormalEvent() ?
                                     monitor.maxTimeOut() : monitor.current();
      //send the heartbeat to the server
      remus::proto::send_Message(remus::common::MeshIOType(),
                                 remus::HEARTBEAT,
                                 boost::lexical_cast<std::string>(polldur),
                                 &serverComm);
      }
    }
}

//------------------------------------------------------------------------------
//presumes the thread is valid
void stopTalking()
{
  this->setIsTalking(false);

  this->PollingThread->join();
}

};


//-----------------------------------------------------------------------------
MessageRouter::MessageRouter(
                const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                const zmq::socketInfo<zmq::proto::inproc>& queue_info):
Implementation( new MessageRouterImplementation(worker_info, queue_info) )
{

}

//-----------------------------------------------------------------------------
MessageRouter::~MessageRouter()
{
}

//-----------------------------------------------------------------------------
bool MessageRouter::valid() const
{
  return this->Implementation->isTalking();
}

//-----------------------------------------------------------------------------
bool MessageRouter::start(const remus::worker::ServerConnection& server_info,
                          zmq::context_t& internal_inproc_context)
{
  return this->Implementation->startTalking(server_info,
                                            internal_inproc_context);
}

}
}
}
