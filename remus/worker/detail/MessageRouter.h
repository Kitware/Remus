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

#ifndef remus_worker_detail_MessageRouter_h
#define remus_worker_detail_MessageRouter_h

#include <remus/proto/zmqSocketInfo.h>
#include <remus/worker/ServerConnection.h>

#include <boost/scoped_ptr.hpp>
#include <string>

namespace remus{
namespace worker{
namespace detail{

//Routes messages from the server to the worker class or the job queue,
//based on the message type. The message router also handles send heartbeat
//message back to the server

//Once a MessageRouter is sent a TerminateWorker message,it will not accept any
//new messages from the Server and trying to start back up the server. Also
//once terminated the MessageRouter can not be restarted.
class MessageRouter
{
public:
  MessageRouter(const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                const zmq::socketInfo<zmq::proto::inproc>& queue_info);

  ~MessageRouter();

  //Returns true when the MessageRouter running and sending messages from
  //the worker to server or server to worker.
  //Only returns false if we are sending no messages in both directions
  bool valid() const;

  //checks to see if the MessageRouter is still forwarding worker messages
  //to the server. When a server stops brokering it tells workers to
  //terminate. When that happens we still want
  bool isForwardingToServer() const;

  //Will return true if the message router can start. Will return false
  //if you try to start a MessageRouter that has been terminated by the server
  //or worker
  bool start(const remus::worker::ServerConnection& server_info,
             zmq::context_t& internal_inproc_context);

private:
  class MessageRouterImplementation;
  boost::scoped_ptr<MessageRouterImplementation> Implementation;

  //make copying not possible
  MessageRouter(const MessageRouter&);
  void operator = (const MessageRouter&);
};

}
}
}

#endif