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

#include <remus/proto/zmqHelper.h>
#include <remus/worker/ServerConnection.h>

#include <boost/scoped_ptr.hpp>
#include <string>

namespace remus{
namespace worker{
namespace detail{

//Routes messages from the server to the worker class or the job queue,
//based on the message type. The message router also handles send heartbeat
//message back to the server
class MessageRouter
{
public:
  MessageRouter(zmq::context_t& context,
                const remus::worker::ServerConnection& server_info,
                const zmq::socketInfo<zmq::proto::inproc>& worker_info,
                const zmq::socketInfo<zmq::proto::inproc>& queue_info);

  ~MessageRouter();

  bool valid() const;

  void start();

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