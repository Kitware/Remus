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
  bool ContinuePolling;

  //Should we continue to forward messages from the worker to the server
  bool ContinueForwardingToServer;
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
  ContinuePolling(false),
  ContinueForwardingToServer(true)
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
  return this->ContinuePolling;
}

//-----------------------------------------------------------------------------
bool isForwardingToServer() const
{
  boost::lock_guard<boost::mutex> lock(ThreadMutex);
  return this->ContinueForwardingToServer;
}

//------------------------------------------------------------------------------
bool startTalking(const remus::worker::ServerConnection& server_info,
                  zmq::context_t& internal_inproc_context)
{
  bool launchThread = false;
  bool threadIsRunning = false;
    //lock the construction of thread as a critical section
    {
    boost::lock_guard<boost::mutex> lock(ThreadMutex);

    //if a thread's id is equal to the default thread id, it means that the
    //thread was never given anything to run, and is actually empty
    threadIsRunning = this->PollingThread->get_id() != boost::thread::id();
    launchThread = !this->ContinuePolling && !threadIsRunning;
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
    boost::lock_guard<boost::mutex> lock(ThreadMutex);
    this->ContinuePolling = t;
    }
  this->ThreadStatusChanged.notify_all();
}

//----------------------------------------------------------------------------
void waitForThreadToStart()
{
  boost::unique_lock<boost::mutex> lock(ThreadMutex);
  while(!this->ContinuePolling)
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
                                        boost::int64_t(60000) );

  //We need to notify the Thread management that polling is about to start.
  //This allows the calling thread to resume, as it has been waiting for this
  //notification, and will also allow threads that have been holding on
  //waitForThreadToStart to resume
  this->setIsTalking(true);
  while( this->isTalking() )
    {
    zmq::poll(&items[0],2,monitor.current());
    monitor.pollOccurred();

    //handle taking
    bool sentToServer = false;
    if(items[1].revents & ZMQ_POLLIN)
        {
        //handle accepting message from the server and forwarding
        //them to the server
        this->handleServerMessage(serverComm, queueComm);
        }
    if(items[0].revents & ZMQ_POLLIN)
        {
        sentToServer = true;
        //handle accepting messages from the worker and forwarding
        //them to the server
        bool shouldSutdown = this->handleWorkerMessage(workerComm,
                                                                                    serverComm,
                                                                                    queueComm);
        if(shouldSutdown)
          {
          //we are shutting down so we mark that we will not accept any
          //more messages from anybody, and instead will stop polling
          this->setIsTalking(false);

          //don't try to handle any message from the server
          //that could be ready
          continue;
          }
        }


     if(!sentToServer)
        {
        //we are going to send a heartbeat now since we have gone long enough
        //without sending a message to the server
        this->sendHeartBeat(serverComm, monitor);
        }
    }
}

//------------------------------------------------------------------------------
//handles taking messages from the worker
bool handleWorkerMessage(zmq::socket_t& workerComm,
                                        zmq::socket_t& serverComm,
                                        zmq::socket_t& queueComm)
{
  bool shouldSutdown = false;
  //first we take the message from the worker socket so it
  //doesn't hang around, and makes the worker think it
  //always is connected to a server
  remus::proto::Message message = remus::proto::receive_Message(&workerComm);

  //next we check if we are forwarding messages to the server,
  //if we aren't doing that there is no point to send the message
  //otherwise it will hang around in our zmq inbox and make
  //use block on destruction of the socket
  if( this->ContinueForwardingToServer )
    {
    //if the worker is telling use to submit a TERMINATE_WORKER
    //job that means it is in the process of shutting down.
    //so we need to prepare for that
    if(message.serviceType()==remus::TERMINATE_WORKER)
      {
      //send the terminate worker message to the job queue so it
      //shuts down
      remus::worker::Job terminateJob;
      remus::proto::Response termJob( remus::TERMINATE_WORKER,
                                       remus::worker::to_string(terminateJob));
      termJob.sendNonBlocking(&queueComm, (zmq::SocketIdentity()) );

      //we are in the process of cleaning up we need to stop everything
      shouldSutdown = true;
      }
    else
      {
      // std::cout << "forward_Message" << this << std::endl;
      //just pass the message on to the server
      remus::proto::forward_Message(message,&serverComm);
      }
    }
    return shouldSutdown;
}

//------------------------------------------------------------------------------
//handles taking messages from the server
void handleServerMessage( zmq::socket_t& serverComm,
                                       zmq::socket_t& queueComm)
{
  remus::proto::Response response(&serverComm);
  const bool goodToForward = response.isFullyFormed() && response.isValidService();
  if(goodToForward)
    {
    switch(response.serviceType())
      {
      case remus::TERMINATE_WORKER:
        response.sendNonBlocking(&queueComm, (zmq::SocketIdentity()));

        //the server has told us to terminate, which means that the server
        //might not exist so don't continue trying to send it messages
        this->ContinueForwardingToServer = false;
        // std::cout << "ContinueForwardingToServer: false" << this <<  std::endl;
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

//------------------------------------------------------------------------------
//handles sending heartbeat to the server
void sendHeartBeat(zmq::socket_t& serverComm,
                             const remus::common::PollingMonitor& monitor)
{
  //First we check if we should be talking to the server, if we aren't forwarding
  //messages to the server than sending heartbeats is pointless
  if( this->ContinueForwardingToServer)
    {
    //send the server how soon in seconds we will send our next heartbeat
    //message. This way we are telling the server itself when it should
    //expect a message, rather than it guessing. We always send the max
    //time out since we don't know how long till we poll again. If a
    //super large job comes in we could be blocking for a very long time
    const boost::int64_t polldur = monitor.maxTimeOut();
    //send the heartbeat to the server
    remus::proto::send_Message(remus::common::MeshIOType(),
                                               remus::HEARTBEAT,
                                               boost::lexical_cast<std::string>(polldur),
                                               &serverComm);
    }
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
bool MessageRouter::isForwardingToServer() const
{
  return this->Implementation->isForwardingToServer();
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
