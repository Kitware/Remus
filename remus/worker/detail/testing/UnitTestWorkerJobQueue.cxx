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

#include <remus/common/SleepFor.h>
#include <remus/proto/Message.h>
#include <remus/proto/Response.h>
#include <remus/worker/detail/JobQueue.h>

#include <remus/proto/zmq.hpp>

#include <remus/testing/Testing.h>



#include <boost/uuid/uuid.hpp>

using namespace remus::worker::detail;

namespace {

void verify_basic_comms(zmq::context_t& context)
{
  zmq::socketInfo<zmq::proto::inproc> queue_channel(
                                                remus::testing::UniqueString());
  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //create the socket to send info to the job queue
  zmq::socket_t jobSocket(context,ZMQ_PAIR);
  jobSocket.connect(queue_channel.endpoint().c_str());

  zmq::SocketIdentity sid;


  boost::uuids::uuid jobId = remus::testing::UUIDGenerator();
  remus::worker::Job fakeJob(jobId,
                             remus::proto::JobSubmission());

  {
  remus::proto::Response response(remus::MAKE_MESH,
                                  remus::worker::to_string(fakeJob));
  response.sendNonBlocking(&jobSocket,sid);
  }

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
  remus::worker::Job fakeJob3(remus::testing::UUIDGenerator(), sub);

  {
  remus::proto::Response response(remus::MAKE_MESH,
                                  remus::worker::to_string(fakeJob2));
  response.sendNonBlocking(&jobSocket,sid);
  }

  {
  remus::proto::Response response(remus::MAKE_MESH,
                                  remus::worker::to_string(fakeJob2));
  response.sendNonBlocking(&jobSocket,sid);
  }

  //now send a terminate job command for the first job
  //and verify that the correct job was terminated by pulling
  //all the jobs off the stack
  {
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());
  remus::proto::Response response(remus::TERMINATE_JOB,
                                  remus::worker::to_string(terminateJob));
  response.sendNonBlocking(&jobSocket, sid);
  }

  //gotta wait for all three messages to come in
  remus::common::SleepForMillisec(2000);

  while(jq.size()<3){}
  REMUS_ASSERT( (jq.size()>0) );
  REMUS_ASSERT( (jq.size()==3) );

  while(jq.size()>0)
    {
    remus::worker::Job j = jq.take();
    if(j.id() == jobId)
      {
      REMUS_ASSERT( (!j.valid()) )
      REMUS_ASSERT( (j.validityReason() ==
                     remus::worker::Job::TERMINATE_WORKER) )
      }
    else
      {
      REMUS_ASSERT( (j.valid()) )
      }
    }
}

void verify_term(zmq::context_t& context)
{
  zmq::socketInfo<zmq::proto::inproc> queue_channel(
                                                remus::testing::UniqueString());
  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //create the socket to send info to the job queue
  zmq::socket_t jobSocket(context,ZMQ_PAIR);
  jobSocket.connect(queue_channel.endpoint().c_str());

  REMUS_ASSERT( (jq.size() == 0) );

  //now send it a terminate message over the worker channel
  remus::worker::Job terminateJob;
    remus::proto::Response response(remus::TERMINATE_WORKER,
                                  remus::worker::to_string(terminateJob));
  response.sendNonBlocking(&jobSocket, (zmq::SocketIdentity()));

  //cheap block while we wait for the router thread to get the message
  while(jq.size()<1){}

  REMUS_ASSERT( (jq.size() == 1) )
  remus::worker::Job invalid_job = jq.take();
  REMUS_ASSERT( (!invalid_job.valid()) )
  REMUS_ASSERT( (invalid_job.validityReason() ==
                 remus::worker::Job::TERMINATE_WORKER) )

}

}

int UnitTestWorkerJobQueue(int, char *[])
{
  zmq::context_t context(1);

  verify_basic_comms(context);
  verify_term(context);

  return 0;
}
