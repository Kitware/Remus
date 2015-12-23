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

#include <remus/server/PortNumbers.h>
#include <remus/common/SleepFor.h>
#include <remus/proto/Message.h>
#include <remus/proto/Response.h>
#include <remus/worker/detail/JobQueue.h>

#include <remus/proto/zmqHelper.h>

#include <remus/testing/Testing.h>

#include <boost/uuid/uuid.hpp>

using namespace remus::worker::detail;

namespace {

//------------------------------------------------------------------------------
remus::worker::ServerConnection bindToTCPSocket(zmq::socket_t &socket)
{
  //try a new port each time we are called this is to help speed up the test
  int port_offset = 94;
  zmq::socketInfo<zmq::proto::tcp> socketInfo("127.0.0.1",
                              remus::server::WORKER_PORT + port_offset);
  socketInfo = zmq::bindToAddress(socket,socketInfo);

  return remus::worker::ServerConnection(socketInfo);
}

//------------------------------------------------------------------------------
void test_worker_terminate_routing_call(MessageRouter& mr,
                                        remus::worker::ServerConnection serverConn,
                                        zmq::socket_t& workerSocket,
                                        JobQueue& jq)
{
  mr.start( serverConn, *(serverConn.context()) );
  REMUS_ASSERT( (mr.valid()) )
  //now send it a terminate message over the worker channel
  remus::proto::Message sent = remus::proto::send_Message(remus::common::MeshIOType(),
                                                          remus::TERMINATE_WORKER,
                                                          &workerSocket);
  REMUS_ASSERT( sent.isValid() )

  //cheap block while we wait for the router thread to get the message
  for(int i=0; i < 10 && jq.size() == 0; ++i)
    {
    remus::common::SleepForMillisec(250);
    }
  REMUS_ASSERT( (jq.size() > 0) )
  remus::common::SleepForMillisec(250);
  REMUS_ASSERT( (!mr.valid()) )

  REMUS_ASSERT( (jq.size() == 1) )
  remus::worker::Job invalid_job = jq.take();
  REMUS_ASSERT( (!invalid_job.valid()) )
  REMUS_ASSERT( (invalid_job.validityReason() ==
                 remus::worker::Job::TERMINATE_WORKER) )
}


}

int UnitTestMessageRouterWorkerTermination(int, char *[])
{
  zmq::socketInfo<zmq::proto::inproc> worker_channel(remus::testing::UniqueString());
  zmq::socketInfo<zmq::proto::inproc> queue_channel(remus::testing::UniqueString());

  //bind the serverSocket
  boost::shared_ptr<zmq::context_t> context = remus::worker::make_ServerContext();
  zmq::socket_t serverSocket(*context, ZMQ_ROUTER);
  remus::worker::ServerConnection serverConn = bindToTCPSocket(serverSocket);
  //set the context on the server connection to the one we just created
  serverConn.context(context);

  //we need to bind to the inproc sockets before constructing the MessageRouter
  //this is a required implementation detail caused by zmq design, also we have
  //to share the same zmq context with the inproc protocol
  zmq::socket_t worker_socket(*context, ZMQ_PAIR);
  zmq::bindToAddress(worker_socket, worker_channel);

  JobQueue jq(*context,queue_channel); //bind the jobqueue to the worker channel

  //It should be noted that once you send a terminate call to a JobQueue
  //or MessageRouter it can't be started again
  MessageRouter mr(worker_channel, queue_channel);
  test_worker_terminate_routing_call(mr,serverConn,worker_socket,jq);
  return 0;
}
