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

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

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

  zmq::socketIdentity sid;

  //now send it a terminate message over the server channel
  remus::proto::Response response(sid);

  boost::uuids::uuid jobId = remus::testing::UUIDGenerator();
  remus::worker::Job fakeJob(jobId,
                             remus::proto::JobSubmission());
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob));
  response.send(jobSocket);

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

  remus::worker::Job fakeJob2(remus::testing::UUIDGenerator(), sub);
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob2));
  response.send(jobSocket);

  remus::worker::Job fakeJob3(remus::testing::UUIDGenerator(), sub);
  response.setServiceType(remus::MAKE_MESH);
  response.setData(remus::worker::to_string(fakeJob2));
  response.send(jobSocket);

  //now send a terminate job command for the first job
  //and verify that the correct job was terminated by pulling
  //all the jobs off the stack
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());
  response.setServiceType(remus::TERMINATE_JOB);
  response.setData(remus::worker::to_string(terminateJob));
  response.send(jobSocket);

  //gotta wait for all three messages to come in
#ifdef _WIN32
      Sleep(2000);
#else
      sleep(2);
#endif
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
  remus::proto::Response response( (zmq::socketIdentity()) );
  remus::worker::Job terminateJob;
  response.setServiceType(remus::TERMINATE_WORKER);
  response.setData(remus::worker::to_string(terminateJob));
  response.send(jobSocket);

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
