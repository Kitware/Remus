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
#include <boost/uuid/random_generator.hpp>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

using namespace remus::worker::detail;

namespace {

boost::uuids::random_generator generator;

void test_job_routing(MessageRouter& mr, zmq::socket_t& socket,
                      JobQueue& jq)
{
  mr.start();
  REMUS_ASSERT( (mr.valid()) )

  //we need to fetch a heartbeat message from the router
  //so that we get the socket identity to send to
  zmq::socketIdentity sid = zmq::address_recv(socket);

  //now send it a terminate message over the server channel
  remus::proto::Response response(sid);

  boost::uuids::uuid jobId = generator();
  remus::worker::Job fakeJob(jobId,
                             remus::proto::JobSubmission());
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob));
  response.send(socket);

  while(jq.size()<1){}
  REMUS_ASSERT( (jq.size()>0) );
  REMUS_ASSERT( (jq.size()==1) );

  //now add multiple more jobs to the queue
  remus::proto::JobRequirements reqs(
          remus::common::ContentSource::Memory,
          remus::common::ContentFormat::User,
          remus::common::MeshIOType( (remus::meshtypes::Mesh2D()),
                                     (remus::meshtypes::Mesh2D()) ),
          std::string(),
          std::string()
          );
  remus::proto::JobSubmission sub(reqs);

  remus::worker::Job fakeJob2(generator(), sub);
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob2));
  response.send(socket);

  remus::worker::Job fakeJob3(generator(), sub);
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob2));
  response.send(socket);

  //now send a terminate job command for the first job
  //and verify that the correct job was terminated by pulling
  //all the jobs off the stack
  remus::worker::Job terminateJob(jobId, sub);
  response.setServiceType(remus::TERMINATE_JOB);
  response.setData(remus::worker::to_string(terminateJob));
  response.send(socket);

  //gotta wait for all three messages to come in
  while(jq.size()<3){}
#ifdef _WIN32
      Sleep(2000);
#else
      sleep(2);
#endif
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
  zmq::socketIdentity sid = zmq::address_recv(socket);

  //now send it a terminate message over the server channel
  remus::proto::Response response(sid);

  remus::worker::Job terminateJob(generator(),
                                  remus::proto::JobSubmission());
  response.setServiceType(remus::TERMINATE_WORKER);
  response.setData(remus::worker::to_string(terminateJob));
  response.send(socket);

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
  shutdown.send(socket);

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

void verify_basic_comms(zmq::context_t& context)
{
  remus::worker::ServerConnection serverConn("127.0.0.1",50511);
  zmq::socketInfo<zmq::proto::inproc> worker_channel("worker");
  zmq::socketInfo<zmq::proto::inproc> queue_channel("jobs");

  //bind the serverSocket
  zmq::socket_t serverSocket(context, ZMQ_ROUTER);
  serverSocket.bind(serverConn.endpoint().c_str());

  //we need to bind to the inproc sockets before constructing the MessageRouter
  //this is a required implementation detail caused by zmq design, also we have
  //to share the same zmq context with the inproc protocol
  zmq::socket_t worker_socket(context, ZMQ_PAIR);
  worker_socket.bind(worker_channel.endpoint().c_str());

  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //now we can construct the message router, and verify that it can
  //be destroyed before starting
  {
  MessageRouter mr(context, serverConn, worker_channel, queue_channel);
  REMUS_ASSERT( (!mr.valid()) )
  }

  //test that we can call start multiple times without issue. It should be
  //noted that since MessageRouter uses ZMQ_PAIR connections we can't have
  //multiple MessageRouters connecting to the same socket, you have to bind
  //and unbind those socket classes.
  MessageRouter mr(context, serverConn, worker_channel, queue_channel);
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

void verify_server_term(zmq::context_t& context)
{
  remus::worker::ServerConnection serverConn("127.0.0.1",50512);
  zmq::socketInfo<zmq::proto::inproc> worker_channel("worker");
  zmq::socketInfo<zmq::proto::inproc> queue_channel("jobs");

  //bind the serverSocket
  zmq::socket_t serverSocket(context, ZMQ_ROUTER);
  serverSocket.bind(serverConn.endpoint().c_str());

  //we need to bind to the inproc sockets before constructing the MessageRouter
  //this is a required implementation detail caused by zmq design, also we have
  //to share the same zmq context with the inproc protocol
  zmq::socket_t worker_socket(context, ZMQ_PAIR);
  worker_socket.bind(worker_channel.endpoint().c_str());

  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //It should be noted that once you send a terminate call to a JobQueue
  //or MessageRouter it can't be started again

  //verify that we can send a TERMINATE_WORKER call from the server properly
  MessageRouter mr(context, serverConn, worker_channel, queue_channel);
  test_server_stop_routing_call(mr,serverSocket,jq);

  REMUS_ASSERT( (mr.start() == false) )
  REMUS_ASSERT( (mr.valid() == false) )
}

void verify_worker_term(zmq::context_t& context)
{
  remus::worker::ServerConnection serverConn("127.0.0.1",50513);
  zmq::socketInfo<zmq::proto::inproc> worker_channel("worker");
  zmq::socketInfo<zmq::proto::inproc> queue_channel("jobs");

  //bind the serverSocket
  zmq::socket_t serverSocket(context, ZMQ_ROUTER);
  serverSocket.bind(serverConn.endpoint().c_str());

  //we need to bind to the inproc sockets before constructing the MessageRouter
  //this is a required implementation detail caused by zmq design, also we have
  //to share the same zmq context with the inproc protocol
  zmq::socket_t worker_socket(context, ZMQ_PAIR);
  worker_socket.bind(worker_channel.endpoint().c_str());

  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //It should be noted that once you send a terminate call to a JobQueue
  //or MessageRouter it can't be started again
  MessageRouter mr(context, serverConn, worker_channel, queue_channel);
  test_worker_stop_routing_call(mr,worker_socket,jq);

  REMUS_ASSERT( (mr.start() == false) )
  REMUS_ASSERT( (mr.valid() == false) )
}

}

int UnitTestMessageRouter(int, char *[])
{
  zmq::context_t context(1);

  verify_basic_comms(context);
  verify_server_term(context);
  verify_worker_term(context);

  return 0;
}
