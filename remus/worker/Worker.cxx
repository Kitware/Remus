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
#include <remus/proto/zmqHelper.h>
#include <remus/worker/detail/JobQueue.h>
#include <remus/worker/detail/MessageRouter.h>

#include <string>

namespace remus{
namespace worker{

//lightweight struct to hide zmq from leaking into libraries that link
//to remus client
namespace detail{
struct ZmqManagement
{
  //use a custom context just for inter process communication, this allows
  //multiple workers to share the same context to the server, as you can't
  //have multiple pairs of ZMQ_PAIR connections on the same named pipe.
  zmq::context_t InterWorkerContext;
  zmq::socket_t Server;
  ZmqManagement():
    InterWorkerContext(1),
    Server(InterWorkerContext, ZMQ_PAIR)
  {
  //We have to bind to the inproc socket before the MessageRouter class does
  std::string ep = zmq::socketInfo<zmq::proto::inproc>("worker").endpoint();
  this->Server.bind( ep.c_str() );
  }
};


}


//-----------------------------------------------------------------------------
Worker::Worker(remus::common::MeshIOType mtype,
               remus::worker::ServerConnection const& conn):
  MeshRequirements( remus::proto::make_JobRequirements(mtype,"","") ),
  ConnectionInfo(conn),
  Zmq( new detail::ZmqManagement() ),
  MessageRouter( new remus::worker::detail::MessageRouter(conn,
                    Zmq->InterWorkerContext,
                    zmq::socketInfo<zmq::proto::inproc>("worker"),
                    zmq::socketInfo<zmq::proto::inproc>("worker_jobs"))),
  JobQueue( new remus::worker::detail::JobQueue( Zmq->InterWorkerContext,
                    zmq::socketInfo<zmq::proto::inproc>("worker_jobs")))
{
  this->MessageRouter->start();

  remus::proto::Message canMesh(this->MeshRequirements.meshTypes(),
                            remus::CAN_MESH,
                            remus::proto::to_string(this->MeshRequirements));
  canMesh.send(&this->Zmq->Server);
}

//-----------------------------------------------------------------------------
Worker::Worker(const remus::proto::JobRequirements& requirements,
               remus::worker::ServerConnection const& conn):
  MeshRequirements(requirements),
  ConnectionInfo(),
  Zmq( new detail::ZmqManagement() ),
  MessageRouter( new remus::worker::detail::MessageRouter(conn,
                    Zmq->InterWorkerContext,
                    zmq::socketInfo<zmq::proto::inproc>("worker"),
                    zmq::socketInfo<zmq::proto::inproc>("worker_jobs")) ),
  JobQueue( new remus::worker::detail::JobQueue( Zmq->InterWorkerContext,
                    zmq::socketInfo<zmq::proto::inproc>("worker_jobs")) )
{
  this->MessageRouter->start();

  remus::proto::Message canMesh(this->MeshRequirements.meshTypes(),
                            remus::CAN_MESH,
                            remus::proto::to_string(this->MeshRequirements));
  canMesh.send(&this->Zmq->Server);
}


//-----------------------------------------------------------------------------
Worker::~Worker()
{
  if(this->MessageRouter->valid())
    {
    //send message that we are shutting down communication, and we can stop
    //polling the server
    remus::proto::Message shutdown(this->MeshRequirements.meshTypes(),
                                   remus::TERMINATE_WORKER);
    shutdown.send(&this->Zmq->Server);
    }
}

//-----------------------------------------------------------------------------
const remus::worker::ServerConnection& Worker::connection() const
{
  return this->ConnectionInfo;
}

//-----------------------------------------------------------------------------
void Worker::askForJobs( unsigned int numberOfJobs )
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

  proto::Message askForMesh(this->MeshRequirements.meshTypes(),
                            remus::MAKE_MESH,
                            proto::to_string(lightReqs));

  for(unsigned int i=0; i < numberOfJobs; ++i)
    { askForMesh.send(&this->Zmq->Server); }
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
  //send a message that contains, the status
  std::string msg = remus::proto::to_string(info);
  remus::proto::Message message(this->MeshRequirements.meshTypes(),
                                remus::MESH_STATUS,
                                msg.data(),msg.size());
  message.send(&this->Zmq->Server);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const remus::proto::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = remus::proto::to_string(result);
  remus::proto::Message message(this->MeshRequirements.meshTypes(),
                                remus::RETRIEVE_MESH,
                                msg.data(),msg.size());
  message.send(&this->Zmq->Server);
}

}
}
