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

#include <remus/worker/Worker.h>

#include <remus/proto/Message.h>
#include <remus/proto/Response.h>
#include <remus/proto/zmqHelper.h>
#include <remus/worker/detail/JobQueue.h>
#include <remus/worker/detail/MessageRouter.h>

#include <string>

//suppress warnings inside boost headers for gcc and clang
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace remus{
namespace worker{

//lightweight struct to hide zmq from leaking into libraries that link
//to remus client
namespace detail{
struct ZmqManagement
{
  //use auto generated channel names, this allows multiple workers to share
  //the same context.
  boost::shared_ptr<zmq::context_t> InterWorkerContext;
  zmq::socket_t Server;
  std::string WorkerChannelUUID;
  std::string JobChannelUUID;
  ZmqManagement( remus::worker::ServerConnection const& conn ):
    InterWorkerContext( conn.context() ),
    Server( *InterWorkerContext, ZMQ_PAIR),
    WorkerChannelUUID(),
    JobChannelUUID()
  {
  boost::uuids::random_generator generator;

  //the goal here is to produce unique socket names. The current solution
  //is to use uuids for the channel names
  WorkerChannelUUID = boost::uuids::to_string(generator());
  JobChannelUUID = boost::uuids::to_string(generator());

  //We have to bind to the inproc socket before the MessageRouter class does
  zmq::socketInfo<zmq::proto::inproc> sInfo( this->WorkerChannelUUID );
  zmq::bindToAddress(this->Server, sInfo);
  }
};


}


//-----------------------------------------------------------------------------
Worker::Worker(remus::common::MeshIOType mtype,
               remus::worker::ServerConnection const& conn):
  MeshRequirements( remus::proto::make_JobRequirements(mtype,"","") ),
  ConnectionInfo(conn),
  Zmq( new detail::ZmqManagement( conn ) ),
  MessageRouter( new remus::worker::detail::MessageRouter(
                    zmq::socketInfo<zmq::proto::inproc>(Zmq->WorkerChannelUUID),
                    zmq::socketInfo<zmq::proto::inproc>(Zmq->JobChannelUUID))),
  JobQueue( new remus::worker::detail::JobQueue( *Zmq->InterWorkerContext,
                    zmq::socketInfo<zmq::proto::inproc>(Zmq->JobChannelUUID)))
{
  //build the buffer before we start the message router. This shortens the
  //duration that the worker is stalling, while the message router is active
  //and sending/recv without a registered worker
  std::string buffer_str;
  {
  std::ostringstream input_buffer;
  input_buffer << this->MeshRequirements;
  buffer_str = input_buffer.str();
  }

  this->MessageRouter->start(conn, *Zmq->InterWorkerContext);
  remus::proto::send_Message(this->MeshRequirements.meshTypes(),
                            remus::CAN_MESH_REQUIREMENTS,
                            buffer_str,
                            &this->Zmq->Server);
}

//-----------------------------------------------------------------------------
Worker::Worker(const remus::proto::JobRequirements& requirements,
               remus::worker::ServerConnection const& conn):
  MeshRequirements(requirements),
  ConnectionInfo(conn),
  Zmq( new detail::ZmqManagement( conn ) ),
  MessageRouter( new remus::worker::detail::MessageRouter(
                    zmq::socketInfo<zmq::proto::inproc>(Zmq->WorkerChannelUUID),
                    zmq::socketInfo<zmq::proto::inproc>(Zmq->JobChannelUUID)) ),
  JobQueue( new remus::worker::detail::JobQueue( *Zmq->InterWorkerContext,
                    zmq::socketInfo<zmq::proto::inproc>(Zmq->JobChannelUUID)) )
{
  //build the buffer before we start the message router. This shortens the
  //duration that the worker is stalling, while the message router is active
  //and sending/recv without a registered worker
  std::string buffer_str;
  {
  std::ostringstream input_buffer;
  input_buffer << this->MeshRequirements;
  buffer_str = input_buffer.str();
  }

  this->MessageRouter->start(conn, *Zmq->InterWorkerContext);
  remus::proto::send_Message(this->MeshRequirements.meshTypes(),
                            remus::CAN_MESH_REQUIREMENTS,
                            buffer_str,
                            &this->Zmq->Server);
}


//-----------------------------------------------------------------------------
Worker::~Worker()
{
  if(this->MessageRouter->valid())
    {
    //send message that we are shutting down communication, and we can stop
    //polling the server
    remus::proto::send_Message(this->MeshRequirements.meshTypes(),
                               remus::TERMINATE_WORKER,
                               &this->Zmq->Server);
    } 
}

//-----------------------------------------------------------------------------
const remus::worker::ServerConnection& Worker::connection() const
{
  return this->ConnectionInfo;
}

//------------------------------------------------------------------------------
void Worker::pollingRates(const remus::worker::PollingRates& rates)
{

  //setup a new polling monitor on the already created MessageRouter.
  //A MessageRouter instance has local state information that needs to exist,
  //so rather than create a new MessageRouter we just changing the existing
  //ones behavior
  this->MessageRouter->pollingMonitor().changeTimeOutRates(rates.minRate(),
                                                           rates.maxRate());
}

//------------------------------------------------------------------------------
remus::worker::PollingRates Worker::pollingRates() const
{
  remus::common::PollingMonitor monitor = this->MessageRouter->pollingMonitor();
  const boost::int64_t low =  monitor.minTimeOut();
  const boost::int64_t high = monitor.maxTimeOut();
  return remus::worker::PollingRates(low,high);
}

//-----------------------------------------------------------------------------
void Worker::askForJobs( unsigned int numberOfJobs )
{
  if(this->MessageRouter->isForwardingToServer())
    {
    //next we send the MAKE_MESH call with the shorter version of the reqs,
    //which have none of the heavy data.
    proto::JobRequirements lightReqs(this->MeshRequirements.formatType(),
                                     this->MeshRequirements.meshTypes(),
                                     this->MeshRequirements.workerName(),
                                     "");
    //override the source type
    lightReqs.SourceType = this->MeshRequirements.sourceType();
    lightReqs.Tag = this->MeshRequirements.tag();

    std::ostringstream input_buffer;
    input_buffer << lightReqs;

    for(unsigned int i=0; i < numberOfJobs; ++i)
      {
      proto::send_Message(this->MeshRequirements.meshTypes(),
                          remus::MAKE_MESH,
                          input_buffer.str(),
                          &this->Zmq->Server);
      }
    }
}

//-----------------------------------------------------------------------------
std::size_t Worker::pendingJobCount() const
{
  return this->JobQueue->size();
}

//-----------------------------------------------------------------------------
remus::worker::Job Worker::takePendingJob()
{
  return this->JobQueue->take();
}

//-----------------------------------------------------------------------------
remus::worker::Job Worker::getJob()
{
  if(this->pendingJobCount() == 0)
    {
    this->askForJobs(1);
    }
  return this->JobQueue->waitAndTakeJob();
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const remus::proto::JobStatus& info)
{
  if(this->MessageRouter->valid())
    {
    //We want to send status as non blocking so we don't waste cycles
    //waiting to hear back from zmq that the message left its inbox
    std::string msg = remus::proto::to_string(info);
    remus::proto::send_NonBlockingMessage(this->MeshRequirements.meshTypes(),
                              remus::MESH_STATUS,
                              msg,
                              &this->Zmq->Server);
    }
}

//-----------------------------------------------------------------------------
void Worker::sendProgress(const remus::worker::Job& j,
                          int progress, const std::string& message)
{
  const remus::proto::JobProgress jp(progress, message);
  remus::proto::JobStatus status(j.id(),jp);
  this->updateStatus(status);
}

//-----------------------------------------------------------------------------
void Worker::sendJobFailure(const remus::worker::Job& j,
                            const std::string& reason)
{
  const remus::proto::JobProgress jp(reason);
  remus::proto::JobStatus status(j.id(),jp);
  //create a status with a message marks us as IN_PROGRESS, so we need to move
  //to a FAILED state.
  status.markAsFailed();
  this->updateStatus(status);
}

//-----------------------------------------------------------------------------
void Worker::returnResult(const remus::proto::JobResult& result)
{
  if(this->MessageRouter->valid())
    {
    //send a message that contains, the path to the resulting file
    std::string msg = remus::proto::to_string(result);
    remus::proto::send_Message(this->MeshRequirements.meshTypes(),
                               remus::RETRIEVE_RESULT,
                               msg,
                               &this->Zmq->Server);

    //we need to block on waiting for the server to notify it has our result.
    //Otherwise it is possible to delete a worker before it is done transimiting
    //really large results to the server. Don't worry if the server terminates
    //the worker before this is over the MessageRouter spoofs the response.
    remus::proto::Response response =
        remus::proto::receive_Response(&this->Zmq->Server);
    (void) response;
    }
}

//-----------------------------------------------------------------------------
bool Worker::workerShouldTerminate() const
{
  return this->JobQueue->isShutdown();
}

//-----------------------------------------------------------------------------
bool Worker::jobShouldBeTerminated(const remus::worker::Job& job) const
{
  return this->JobQueue->isATerminatedJob(job);
}

}
}
