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
#include <boost/thread.hpp>

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
  boost::mutex ThreadMutex;
  boost::scoped_ptr<boost::thread> PollingThread;
  //state to tell when we should stop polling
  bool ContinueTalking;
public:
//-----------------------------------------------------------------------------
MessageRouterImplementation(zmq::context_t& context,
                      const remus::worker::ServerConnection& server_info,
                      const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                      const zmq::socketInfo<zmq::proto::inproc>& queue_info):
  WorkerComm(context,ZMQ_PAIR),
  QueueComm(context,ZMQ_PAIR),
  ServerComm(context,ZMQ_DEALER),
  WorkerEndpoint(worker_info.endpoint()),
  QueueEndpoint(queue_info.endpoint()),
  ServerEndpoint(server_info.endpoint()),
  ThreadMutex(),
  PollingThread(NULL),
  ContinueTalking(false)
{
  //we don't connect the sockets until the polling thread starts up
}

//-----------------------------------------------------------------------------
~MessageRouterImplementation()
{
  this->PollingThread->join();
}

//-----------------------------------------------------------------------------
bool isTalking() const
{
  boost::unique_lock<boost::mutex>(ThreadMutex);
  return this->ContinueTalking;
}

//------------------------------------------------------------------------------
void stopTalking()
{
  boost::unique_lock<boost::mutex>(ThreadMutex);
  this->ContinueTalking = false;
}

//------------------------------------------------------------------------------
void startTalking()
{
  zmq::connectToAddress(this->ServerComm, this->ServerEndpoint);
  zmq::connectToAddress(this->WorkerComm, this->WorkerEndpoint);
  zmq::connectToAddress(this->QueueComm,  this->QueueEndpoint);

  zmq::pollitem_t items[2]  = {
                                { this->WorkerComm,  0, ZMQ_POLLIN, 0 },
                                { this->ServerComm,  0, ZMQ_POLLIN, 0 }
                              };
  { //start talking
  boost::unique_lock<boost::mutex>(ThreadMutex);
  this->ContinueTalking = true;
  }

  while( this->isTalking() )
    {
    zmq::poll(&items[0],2,remus::HEARTBEAT_INTERVAL);
    bool sentToServer=false;
    if(items[0].revents & ZMQ_POLLIN)
      {
      sentToServer = true;
      remus::proto::Message message(this->WorkerComm);

      //just pass the message on to the server
      message.send(this->ServerComm);

      //special case is that TERMINATE_JOB_AND_WORKER means we stop looping
      if(message.serviceType()==remus::TERMINATE_WORKER)
        {
        this->stopTalking();
        }
      }
    if(items[1].revents & ZMQ_POLLIN && this->isTalking())
      {
      remus::proto::Response response(this->ServerComm);
      switch(response.serviceType())
          {
          case remus::TERMINATE_WORKER:
            this->stopTalking();
          case remus::TERMINATE_JOB:
            //send the terminate to the job queue since it holds the jobs
          case remus::MAKE_MESH:
            //send the job to the queue so that somebody can take it later
            response.send(this->QueueComm);
            break;
          default:
            response.send(this->WorkerComm);
          }
      }
    if(!sentToServer)
      {
      //send a heartbeat to the server
      remus::proto::Message message(remus::common::MeshIOType(),
                                    remus::HEARTBEAT);
      message.send(this->ServerComm);
      }
    }
}

};


//-----------------------------------------------------------------------------
MessageRouter::MessageRouter(zmq::context_t& context,
                const remus::worker::ServerConnection& server_info,
                const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                const zmq::socketInfo<zmq::proto::inproc>& queue_info):
Implementation( new MessageRouterImplementation(context, server_info,
                                                worker_info, queue_info) )
{

}

//-----------------------------------------------------------------------------
MessageRouter::~MessageRouter()
{
  this->Implementation->stopTalking();
}

//-----------------------------------------------------------------------------
bool MessageRouter::valid() const
{
  return this->Implementation->isTalking();
}

//-----------------------------------------------------------------------------
void MessageRouter::start()
{
  this->Implementation->startTalking();
}

}
}
}
