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
  std::size_t OutstandingResults;

  //kept as a member variable so that we can allow the user to specify
  //custom polling rates for workers
  remus::common::PollingMonitor PollMonitor;

  //thread our polling method
  mutable boost::mutex ThreadMutex;
  boost::condition_variable ThreadStatusChanged;

  boost::scoped_ptr<boost::thread> PollingThread;

  //state to tell when we should stop polling
  bool ContinuePolling;

  //Should we continue to forward messages from the worker to the server
  bool ContinueForwardingToServer;

  //Should we continue to forward messages from the server to the worker
  bool ContinueForwardingToWorker;

public:
//-----------------------------------------------------------------------------
MessageRouterImplementation(
                      const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                      const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  WorkerEndpoint(worker_info.endpoint()),
  QueueEndpoint(queue_info.endpoint()),
  OutstandingResults(0),
  PollMonitor(boost::int64_t(250), boost::int64_t(60000)), //assign a low floor for faster testing
  ThreadMutex(),
  ThreadStatusChanged(),
  PollingThread(new boost::thread()),
  ContinuePolling(false),
  ContinueForwardingToServer(true),
  ContinueForwardingToWorker(true)
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

//------------------------------------------------------------------------------
remus::common::PollingMonitor monitor() const { return PollMonitor; }

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


  //We need to notify the Thread management that polling is about to start.
  //This allows the calling thread to resume, as it has been waiting for this
  //notification, and will also allow threads that have been holding on
  //waitForThreadToStart to resume
  this->setIsTalking(true);
  while( this->isTalking() )
    {
    zmq::poll(&items[0],2,this->PollMonitor.current());
    this->PollMonitor.pollOccurred();

    //handle taking
    bool notSentToServer = true;
    if(items[1].revents & ZMQ_POLLIN)
        {
        //handle accepting message from the server and forwarding
        //them to the server. Handle server messages before worker
        //messages so that we don't send messages to a server
        //that is now telling us to shut down
        this->handleServerMessage(workerComm, serverComm, queueComm);
        }
    if(items[0].revents & ZMQ_POLLIN)
        {
        notSentToServer = false;
        //handle accepting messages from the worker and forwarding
        //them to the server
        this->handleWorkerMessage(workerComm, serverComm, queueComm);
        if(!ContinueForwardingToWorker)
          {
          //we are shutting down so we mark that we will not accept any
          //more messages from anybody, and instead will stop polling
          this->setIsTalking(false);
          }
        }

     if(notSentToServer)
        {
        //we are going to send a heartbeat now since we have gone long enough
        //without sending a message to the server
        this->sendHeartBeat(serverComm, this->PollMonitor);
        }
    }
}

//------------------------------------------------------------------------------
//handles taking messages from the worker
void handleWorkerMessage(zmq::socket_t& workerComm,
                         zmq::socket_t& serverComm,
                         zmq::socket_t& queueComm)
{
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
    //first we need to forward all message to the server
    remus::proto::forward_Message(message,&serverComm);

    //if the worker is telling use to submit a TERMINATE_WORKER
    //job that means it is in the process of shutting down.
    //so we need to prepare for that
    if(message.serviceType()==remus::TERMINATE_WORKER)
      {
      //send the terminate worker message to the job queue so it
      //shuts down. This has to be blocking so we know that it has
      //received the message.
      remus::worker::Job terminateJob;
      remus::proto::send_Response(remus::TERMINATE_WORKER,
                                  remus::worker::to_string(terminateJob),
                                  &queueComm,
                                  (zmq::SocketIdentity()) );

      //we are in the process of cleaning up we need to stop everything.
      //we first check if we have any outstanding job results that
      //are waiting around to be sent
      this->ContinueForwardingToWorker = false;
      }
    else if(message.serviceType()==remus::RETRIEVE_RESULT)
      {
      //Mark that we need a response from the server, this is required so that
      //we can send back really large result data. When we don't wait for the
      //server to get our data, we will drop the results as the socket linger
      //time is less than the amount of time it takes to transmit the results
      //to the server.
      ++this->OutstandingResults;
      }
    }
}

//------------------------------------------------------------------------------
//handles taking messages from the server
void handleServerMessage( zmq::socket_t& workerComm,
                          zmq::socket_t& serverComm,
                          zmq::socket_t& queueComm)
{
  remus::proto::Response response = remus::proto::receive_Response(&serverComm);
  const bool goodToForward = response.isValid();

  //determine if we can send onto the job queue
  const bool goodToForwardToQueue = goodToForward &&
                                    this->ContinueForwardingToWorker;
  if(goodToForward)
    {
    if(goodToForwardToQueue &&
       response.serviceType() == remus::TERMINATE_WORKER)
      {
      //if the server is shutting down the worker and the worker
      //is still waiting for a response to a RETRIEVE_RESULT we
      //send that first
      while(this->OutstandingResults > 0)
        {
        remus::proto::send_NonBlockingResponse(remus::RETRIEVE_RESULT,
                                               remus::INVALID_MSG,
                                               &workerComm,
                                               (zmq::SocketIdentity()));
        --this->OutstandingResults;
        }
      remus::proto::forward_Response(response,
                                     &queueComm,
                                     zmq::SocketIdentity());

      //the server has told us to terminate, which means that the server
      //might not exist so don't continue trying to send it messages
      this->ContinueForwardingToServer = false;
      }
    else if(goodToForwardToQueue &&
            ( response.serviceType() == remus::TERMINATE_JOB ||
              response.serviceType() == remus::MAKE_MESH ) )
      {
      remus::proto::forward_Response(response,
                                     &queueComm,
                                     zmq::SocketIdentity());
      }
    else if ( response.serviceType() == remus::RETRIEVE_RESULT)
      { //the worker is notifying us that it recieved our results, so decrement
        //our outstanding results, and forward the message to the worker so
        //it can stop blocking
      remus::proto::forward_Response(response,
                                     &workerComm,
                                     zmq::SocketIdentity());
        --this->OutstandingResults;
      }
      // do nothing if it isn't terminate_job, terminate_worker,
      // make_mesh or retrieve result
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

//-----------------------------------------------------------------------------
remus::common::PollingMonitor MessageRouter::pollingMonitor() const
{
  return this->Implementation->monitor();
}

}
}
}
