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
#include <remus/worker/detail/JobQueue.h>

#include <remus/proto/zmq.hpp>

#include <remus/testing/Testing.h>

#include <boost/uuid/uuid.hpp>

using namespace remus::worker::detail;

namespace {

remus::worker::ServerConnection bindToTCPSocket(zmq::socket_t &socket)
{
  //try a new port each time we are called this is to help speed up the test
  static int port_offset = 0;
  ++port_offset;

  int rc = -1;
  zmq::socketInfo<zmq::proto::tcp> socketInfo("127.0.0.1",
                                    port_offset + remus::SERVER_WORKER_PORT);
  for(int i=socketInfo.port();i < 65535 && rc != 0; ++i)
    {
    socketInfo.setPort(i);
    //using the C syntax to skip having to catch the exception;
    rc = zmq_bind(socket.operator void *(),socketInfo.endpoint().c_str());
    }

  if(rc!=0)
    {
    throw zmq::error_t();
    }
  return remus::worker::ServerConnection(socketInfo);
}

void test_job_routing(MessageRouter& mr, zmq::socket_t& socket,
                      JobQueue& jq)
{
  mr.start();
  REMUS_ASSERT( (mr.valid()) )

  //we need to fetch a heartbeat message from the router
  //so that we get the socket identity to send to
  zmq::SocketIdentity sid = zmq::address_recv(socket);

  //now send it a terminate message over the server channel
  remus::proto::Response response(sid);

  boost::uuids::uuid jobId = remus::testing::UUIDGenerator();
  remus::worker::Job fakeJob(jobId,
                             remus::proto::JobSubmission());
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob));
  response.send(&socket);

  while(jq.size()<1){}
  REMUS_ASSERT( (jq.size()>0) );
  REMUS_ASSERT( (jq.size()==1) );

  //now add multiple more jobs to the queue
  remus::proto::JobRequirements reqs(
          remus::common::ContentFormat::User,
          remus::common::MeshIOType( (remus::meshtypes::Mesh2D()),
                                     (remus::meshtypes::Mesh2D()) ),
          std::string(),
          std::string()
          );
  remus::proto::JobSubmission sub(reqs);

  remus::worker::Job fakeJob2(remus::testing::UUIDGenerator(), sub);
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob2));
  response.send(&socket);

  remus::worker::Job fakeJob3(remus::testing::UUIDGenerator(), sub);
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob2));
  response.send(&socket);

  //now send a terminate job command for the first job
  //and verify that the correct job was terminated by pulling
  //all the jobs off the stack
  remus::worker::Job terminateJob(jobId, sub);
  response.setServiceType(remus::TERMINATE_JOB);
  response.setData(remus::worker::to_string(terminateJob));
  response.send(&socket);

  //gotta wait for all three messages to come in
  while(jq.size()<3){}

  remus::testing::sleepForMillisec(2000);

  REMUS_ASSERT( (jq.size()>0) );
  REMUS_ASSERT( (jq.size()==3) );

  while(jq.size()>0)
    {
    remus::worker::Job j = jq.take();
    if(j.id() == jobId)
      {
      REMUS_ASSERT( (!j.valid()) )
      REMUS_ASSERT( (j.validityReason() == remus::worker::Job::INVALID) )
      }
    else
      {
      REMUS_ASSERT( (j.valid()) )
      }
    }
}

void test_server_stop_routing_call(MessageRouter& mr, zmq::socket_t& socket,
                                   JobQueue& jq)
{
  mr.start();
  REMUS_ASSERT( (mr.valid()) )

  //we need to fetch a heartbeat message from the router
  //so that we get the socket identity to send to
  zmq::SocketIdentity sid = zmq::address_recv(socket);

  //now send it a terminate message over the server channel
  remus::proto::Response response(sid);

  remus::worker::Job terminateJob(remus::testing::UUIDGenerator(),
                                  remus::proto::JobSubmission());
  response.setServiceType(remus::TERMINATE_WORKER);
  response.setData(remus::worker::to_string(terminateJob));
  response.send(&socket);

  //cheap block while we wait for the router thread to get the message
  while(jq.size()<1){}
  while(mr.valid()){}
  REMUS_ASSERT( (!mr.valid()) )

  REMUS_ASSERT( (jq.size() == 1) )
  remus::worker::Job invalid_job = jq.take();
  REMUS_ASSERT( (!invalid_job.valid()) )
  REMUS_ASSERT( (invalid_job.validityReason() ==
                 remus::worker::Job::TERMINATE_WORKER) )
}

void test_worker_stop_routing_call(MessageRouter& mr, zmq::socket_t& socket,
                                   JobQueue& jq)
{
  mr.start();
  REMUS_ASSERT( (mr.valid()) )

  //now send it a terminate message over the worker channel
  remus::proto::Message shutdown(remus::common::MeshIOType(),
                                 remus::TERMINATE_WORKER);
  shutdown.send(&socket);

  //cheap block while we wait for the router thread to get the message
  while(jq.size()<1){}
  while(mr.valid()){}
  REMUS_ASSERT( (!mr.valid()) )

  REMUS_ASSERT( (jq.size() == 1) )
  remus::worker::Job invalid_job = jq.take();
  REMUS_ASSERT( (!invalid_job.valid()) )
  REMUS_ASSERT( (invalid_job.validityReason() ==
                 remus::worker::Job::TERMINATE_WORKER) )
}

void verify_basic_comms()
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
  worker_socket.bind(worker_channel.endpoint().c_str());

  JobQueue jq(*context,queue_channel); //bind the jobqueue to the worker channel

  //now we can construct the message router, and verify that it can
  //be destroyed before starting
  {
  MessageRouter mr(serverConn, *(serverConn.context()),
                   worker_channel, queue_channel);
  REMUS_ASSERT( (!mr.valid()) )
  }

  //test that we can call start multiple times without issue. It should be
  //noted that since MessageRouter uses ZMQ_PAIR connections we can't have
  //multiple MessageRouters connecting to the same socket, you have to bind
  //and unbind those socket classes.
  MessageRouter mr(serverConn, *(serverConn.context()),
                   worker_channel, queue_channel);
  {
  REMUS_ASSERT( (!mr.valid()) )
  mr.start();
  REMUS_ASSERT( (mr.valid()) )
  mr.start();
  mr.start();
  mr.start();
  mr.start();
  REMUS_ASSERT( (mr.valid()) )
  }

  //verify that we route messages to the job queue
  test_job_routing(mr,serverSocket,jq);
}

void verify_server_term()
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
  worker_socket.bind(worker_channel.endpoint().c_str());

  JobQueue jq(*context,queue_channel); //bind the jobqueue to the worker channel

  //It should be noted that once you send a terminate call to a JobQueue
  //or MessageRouter it can't be started again

  //verify that we can send a TERMINATE_WORKER call from the server properly
    MessageRouter mr(serverConn, *(serverConn.context()),
                   worker_channel, queue_channel);
  test_server_stop_routing_call(mr,serverSocket,jq);

  REMUS_ASSERT( (mr.start() == false) )
  REMUS_ASSERT( (mr.valid() == false) )
}

void verify_worker_term()
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
  worker_socket.bind(worker_channel.endpoint().c_str());

  JobQueue jq(*context,queue_channel); //bind the jobqueue to the worker channel

  //It should be noted that once you send a terminate call to a JobQueue
  //or MessageRouter it can't be started again
  MessageRouter mr(serverConn, *(serverConn.context()),
                   worker_channel, queue_channel);
  test_worker_stop_routing_call(mr,worker_socket,jq);

  REMUS_ASSERT( (mr.start() == false) )
  REMUS_ASSERT( (mr.valid() == false) )
}

}

int UnitTestMessageRouter(int, char *[])
{
  std::cout << "verify_basic_comms" << std::endl;
  verify_basic_comms();

  std::cout << "verify_server_term" << std::endl;
  verify_server_term();

  std::cout << "verify_worker_term" << std::endl;
  verify_worker_term();

  return 0;
}
