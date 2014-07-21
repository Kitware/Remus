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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <boost/thread.hpp>
#pragma GCC diagnostic pop

#include <boost/thread/locks.hpp>
#include <boost/uuid/uuid.hpp>

namespace remus{
namespace worker{
namespace detail{

//-----------------------------------------------------------------------------
class MessageRouter::MessageRouterImplementation
{
  zmq::socket_t WorkerComm;
  zmq::socket_t QueueComm;
  zmq::socket_t ServerComm;

  std::string WorkerEndpoint;
  std::string QueueEndpoint;
  std::string ServerEndpoint;

  //thread our polling method
  mutable boost::mutex ThreadMutex;
  boost::scoped_ptr<boost::thread> PollingThread;
  //state to tell when we should stop polling
  bool ContinueTalking;
public:
//-----------------------------------------------------------------------------
MessageRouterImplementation(
                      zmq::context_t& internal_inproc_context,
                      const remus::worker::ServerConnection& server_info,
                      const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                      const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  //use a custom context just for inter process communication, this allows
  //multiple workers to share the same context to the server
  WorkerComm(internal_inproc_context,ZMQ_PAIR),
  QueueComm(internal_inproc_context,ZMQ_PAIR),
  ServerComm(*(server_info.context()),ZMQ_DEALER),
  WorkerEndpoint(worker_info.endpoint()),
  QueueEndpoint(queue_info.endpoint()),
  ServerEndpoint(server_info.endpoint()),
  ThreadMutex(),
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
    this->stopTalking();
    }

}

//-----------------------------------------------------------------------------
bool isTalking() const
{
  boost::lock_guard<boost::mutex> lock(ThreadMutex);
  return this->ContinueTalking;
}

//------------------------------------------------------------------------------
bool startTalking()
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
      zmq::connectToAddress(this->ServerComm, this->ServerEndpoint);
      zmq::connectToAddress(this->WorkerComm, this->WorkerEndpoint);
      zmq::connectToAddress(this->QueueComm,  this->QueueEndpoint);

      this->ContinueTalking = true;

      boost::scoped_ptr<boost::thread> pollthread(
        new boost::thread( &MessageRouterImplementation::poll, this) );
      this->PollingThread.swap(pollthread);
      }
    }

  //do this outside the previous critical section so that we properly
  //tell other threads that the thread has been launched. We don't
  //want to cause a recursive lock in the same thread to happen
  if(launchThread)
    {
    this->setIsTalking(true);
    }

  //we can only launch the thread once, and once it has been terminated
  //the entire message router is invalid to start up again.
  return launchThread;
}

private:

//----------------------------------------------------------------------------
void setIsTalking(bool t)
{
    {
    boost::unique_lock<boost::mutex> lock(ThreadMutex);
    this->ContinueTalking = t;
    }
}

//------------------------------------------------------------------------------
void poll()
{
  zmq::pollitem_t items[2]  = {
                                { this->WorkerComm,  0, ZMQ_POLLIN, 0 },
                                { this->ServerComm,  0, ZMQ_POLLIN, 0 }
                              };

  remus::common::PollingMonitor monitor;
  while( this->isTalking() )
    {
    bool sentToServer=false;
    zmq::poll(&items[0],2,monitor.current());
    monitor.pollOccurred();

    if(items[0].revents & ZMQ_POLLIN)
      {
      sentToServer = true;

      remus::proto::Message message(&this->WorkerComm);

      //special case is that TERMINATE_WORKER means we stop looping
      if(message.serviceType()==remus::TERMINATE_WORKER)
        {
        //send the terminate worker message to the job queue too
        remus::worker::Job terminateJob;
        remus::proto::Response termJob( remus::TERMINATE_WORKER,
                                        remus::worker::to_string(terminateJob));
        termJob.sendNonBlocking(&this->QueueComm, (zmq::SocketIdentity()) );

        this->setIsTalking(false);
        }
      else
        {
        //just pass the message on to the server
        message.send(&this->ServerComm);
        }
      }
    if(items[1].revents & ZMQ_POLLIN)
      {
      remus::proto::Response response(&this->ServerComm);
      const bool goodToForward = response.isFullyFormed() &&
                                 response.isValidService();
      if(goodToForward)
        {
        switch(response.serviceType())
          {
          case remus::TERMINATE_WORKER:
            response.sendNonBlocking(&this->QueueComm, (zmq::SocketIdentity()));
            this->setIsTalking(false);
            break;
          case remus::TERMINATE_JOB:
            //send the terminate to the job queue since it holds the jobs
          case remus::MAKE_MESH:
            //send the job to the queue so that somebody can take it later
            response.sendNonBlocking(&this->QueueComm, (zmq::SocketIdentity()));
            break;
          default:
            // do nothing if it isn't terminate_job, terminate_worker
            // or make_mesh.
            // response.send(&this->WorkerComm);
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
      remus::proto::Message message(remus::common::MeshIOType(),
                                    remus::HEARTBEAT,
                                    boost::lexical_cast<std::string>(polldur));
      message.send(&this->ServerComm);
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
                const remus::worker::ServerConnection& server_info,
                zmq::context_t& internal_inproc_context,
                const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                const zmq::socketInfo<zmq::proto::inproc>& queue_info):
Implementation( new MessageRouterImplementation(internal_inproc_context,
                                                server_info,
                                                worker_info,
                                                queue_info) )
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
bool MessageRouter::start()
{
  return this->Implementation->startTalking();
}

}
}
}
