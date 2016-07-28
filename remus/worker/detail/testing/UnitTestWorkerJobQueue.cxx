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

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

using namespace remus::worker::detail;

namespace {

//------------------------------------------------------------------------------
void verify_basic_comms(zmq::context_t& context)
{
  zmq::socketInfo<zmq::proto::inproc> queue_channel(
                                                remus::testing::UniqueString());
  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //we need to pause for some time to all the JobQueue
  //to properly bind to the socket before we connect
  while(!jq.isReady())
    { remus::common::SleepForMillisec(10); }

  //create the socket to send info to the job queue
  zmq::socket_t jobSocket(context,ZMQ_PAIR);
  jobSocket.connect(queue_channel.endpoint().c_str());

  zmq::SocketIdentity sid;


  boost::uuids::uuid jobId = remus::testing::UUIDGenerator();
  remus::worker::Job fakeJob(jobId,
                             remus::proto::JobSubmission());

  {
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                             remus::worker::to_string(fakeJob),
                                             &jobSocket,
                                             sid);
  REMUS_ASSERT( (r.isValid()) );
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
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                             remus::worker::to_string(fakeJob2),
                                             &jobSocket,
                                             sid);
  REMUS_ASSERT( (r.isValid()) );
  }

  {
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                            remus::worker::to_string(fakeJob3),
                                            &jobSocket,
                                            sid);
  REMUS_ASSERT( (r.isValid()) );
  }

  //now send a terminate job command for the first job
  //and verify that the correct job was terminated by pulling
  //all the jobs off the stack
  {
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());
  remus::proto::Response r =
      remus::proto::send_NonBlockingResponse(remus::TERMINATE_JOB,
                                             remus::worker::to_string(terminateJob),
                                             &jobSocket,
                                             sid);
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
void verify_term(zmq::context_t& context)
{
  zmq::socketInfo<zmq::proto::inproc> queue_channel(
                                                remus::testing::UniqueString());
  JobQueue jq(context,queue_channel); //bind the jobqueue to the worker channel

  //we need to pause for some time to all the JobQueue
  //to properly bind to the socket before we connect
  while(!jq.isReady())
    { remus::common::SleepForMillisec(10); }

  //create the socket to send info to the job queue
  zmq::socket_t jobSocket(context,ZMQ_PAIR);
  jobSocket.connect(queue_channel.endpoint().c_str());

  REMUS_ASSERT( (jq.size() == 0) );

  //now send it a terminate message over the worker channel
  remus::worker::Job terminateJob;
  remus::proto::Response r=
      remus::proto::send_NonBlockingResponse(remus::TERMINATE_WORKER,
                                             remus::worker::to_string(terminateJob),
                                             &jobSocket,
                                             (zmq::SocketIdentity()));
  REMUS_ASSERT( (r.isValid()) );

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
