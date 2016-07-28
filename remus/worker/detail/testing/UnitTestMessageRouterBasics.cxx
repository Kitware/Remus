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

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

using namespace remus::worker::detail;

namespace {

//------------------------------------------------------------------------------
remus::worker::ServerConnection bindToTCPSocket(zmq::socket_t &socket)
{
  //try a new port each time we are called this is to help speed up the test
  int port_offset = 52;
  zmq::socketInfo<zmq::proto::tcp> socketInfo("127.0.0.1",
                              remus::server::WORKER_PORT + port_offset);
  socketInfo = zmq::bindToAddress(socket,socketInfo);

  return remus::worker::ServerConnection(socketInfo);
}

//------------------------------------------------------------------------------
void test_job_routing(MessageRouter& mr,
                      remus::worker::ServerConnection serverConn,
                      zmq::socket_t& serverSocket,
                      JobQueue& jq)
{
  mr.start( serverConn, *(serverConn.context()) );
  REMUS_ASSERT( (mr.valid()) )

  //we need to fetch a heartbeat message from the router
  //so that we get the socket identity to send to
  zmq::SocketIdentity sid = zmq::address_recv(serverSocket);


  boost::uuids::uuid jobId = remus::testing::UUIDGenerator();
  remus::worker::Job fakeJob(jobId,
                             remus::proto::JobSubmission());

  //now send it a terminate message over the server channel
  {
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                             remus::worker::to_string(fakeJob),
                                             &serverSocket,
                                             sid);
  REMUS_ASSERT( (r.isValid()) );
  }

  while(jq.size()<1)
    {
    remus::common::SleepForMillisec(150);
    }
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
  {
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                             remus::worker::to_string(fakeJob2),
                                             &serverSocket,sid);
  REMUS_ASSERT( (r.isValid()) );
  }

  remus::worker::Job fakeJob3(remus::testing::UUIDGenerator(), sub);
  {
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                             remus::worker::to_string(fakeJob3),
                                             &serverSocket,sid);
  REMUS_ASSERT( (r.isValid()) );
  }

  //now send a terminate job command for the first job
  //and verify that the correct job was terminated by pulling
  //all the jobs off the stack
  {
  remus::worker::Job terminateJob(jobId, sub);
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::TERMINATE_JOB,
                                             remus::worker::to_string(terminateJob),
                                             &serverSocket,sid);
  REMUS_ASSERT( (r.isValid()) );
  }

  //gotta wait for all three messages to come in
  remus::common::SleepForMillisec(2000);

  REMUS_ASSERT( (jq.isATerminatedJob( fakeJob ) ==true ))
  REMUS_ASSERT( (jq.isATerminatedJob( fakeJob2 ) ==false ))
  REMUS_ASSERT( (jq.isATerminatedJob( fakeJob3) ==false ))

  while(jq.size()>0)
    {
    remus::worker::Job j = jq.take();
    REMUS_ASSERT( (j.valid() == true) )
    }
}

//------------------------------------------------------------------------------
void test_polling_rates(MessageRouter& mr)
{
  //verify that we get valid numbers for the polling rates by default
  remus::common::PollingMonitor monitor = mr.pollingMonitor();
  REMUS_ASSERT( (monitor.minTimeOut() > 0) )
  REMUS_ASSERT( (monitor.minTimeOut() < monitor.maxTimeOut()) )

  //verify that we can't pass negative polling values, but instead
  //clamp to zero
  monitor.changeTimeOutRates(-4, -20);
  REMUS_ASSERT( (mr.pollingMonitor().minTimeOut() == 0 ) )
  REMUS_ASSERT( (mr.pollingMonitor().maxTimeOut() == 0 ) )

  //verify that we properly invert polling numbers
  monitor.changeTimeOutRates(400, 20);
  REMUS_ASSERT( (mr.pollingMonitor().minTimeOut() == 20 ) )
  REMUS_ASSERT( (mr.pollingMonitor().maxTimeOut() == 400 ) )

  //verify that we properly invert polling numbers and handle
  //negative and the same time
  monitor.changeTimeOutRates(100, -20);
  REMUS_ASSERT( (mr.pollingMonitor().minTimeOut() == 0 ) )
  REMUS_ASSERT( (mr.pollingMonitor().maxTimeOut() == 100 ) )

  //verify that valid polling rates are kept by the worker
  monitor.changeTimeOutRates(30,120);
  REMUS_ASSERT( (mr.pollingMonitor().minTimeOut() == 30 ) )
  REMUS_ASSERT( (mr.pollingMonitor().maxTimeOut() == 120 ) )
}

}

int UnitTestMessageRouterBasics(int, char *[])
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

  //now we can construct the message router, and verify that it can
  //be destroyed before starting
  {
  MessageRouter mr(worker_channel, queue_channel);
  REMUS_ASSERT( (!mr.valid()) )
  }

  //test that we can call start multiple times without issue. It should be
  //noted that since MessageRouter uses ZMQ_PAIR connections we can't have
  //multiple MessageRouters connecting to the same socket, you have to bind
  //and unbind those socket classes.
  MessageRouter mr(worker_channel, queue_channel);
  {
  REMUS_ASSERT( (!mr.valid()) )
  mr.start( serverConn, *(serverConn.context()) );
  REMUS_ASSERT( (mr.valid()) )
  mr.start( serverConn, *(serverConn.context()) );
  mr.start( serverConn, *(serverConn.context()) );
  REMUS_ASSERT( (mr.valid()) )
  }

  //verify that we route messages to the job queue
  test_job_routing(mr,serverConn,serverSocket,jq);


  //verify that we can change the polling rages of the Message Router
  {
  MessageRouter invalid_mr(worker_channel, queue_channel);
  test_polling_rates(invalid_mr);

  test_polling_rates(mr);
  }
  return 0;
}
