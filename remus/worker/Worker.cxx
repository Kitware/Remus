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

//-----------------------------------------------------------------------------
Worker::Worker(remus::common::MeshIOType mtype,
               remus::worker::ServerConnection const& conn):
  MeshIOType(mtype),
  ConnectionInfo(conn),
  Context(1),
  ServerSocket(Context,ZMQ_PAIR),
  MessageRouter( new remus::worker::detail::MessageRouter(Context, conn,
                    zmq::socketInfo<zmq::proto::inproc>("worker"),
                    zmq::socketInfo<zmq::proto::inproc>("worker_jobs")) ),
  JobQueue( new remus::worker::detail::JobQueue(Context,
                    zmq::socketInfo<zmq::proto::inproc>("worker_jobs")) )
{
  //We have to bind to the inproc socket before the MessageRouter class does
  zmq::bindToAddress<zmq::proto::inproc>(this->ServerSocket,"worker");

  this->MessageRouter->start();

  remus::proto::Message canMesh(this->MeshIOType,remus::CAN_MESH);
  canMesh.send(this->ServerSocket);
}

//-----------------------------------------------------------------------------
Worker::~Worker()
{
  if(this->MessageRouter->valid())
    {
    //send message that we are shutting down communication, and we can stop
    //polling the server
    remus::proto::Message shutdown(this->MeshIOType,
                                   remus::TERMINATE_WORKER);
    shutdown.send(this->ServerSocket);
    }
}

//-----------------------------------------------------------------------------
void Worker::askForJobs( unsigned int numberOfJobs )
{
  remus::proto::Message askForMesh(this->MeshIOType,remus::MAKE_MESH);
  for(int i=0; i < numberOfJobs; ++i)
    { askForMesh.send(this->ServerSocket); }
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
    while(this->pendingJobCount() == 0) {}
    }
  return this->JobQueue->take();
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const remus::proto::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = remus::proto::to_string(info);
  remus::proto::Message message(this->MeshIOType,
                                    remus::MESH_STATUS,
                                    msg.data(),msg.size());
  message.send(this->ServerSocket);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const remus::proto::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = remus::proto::to_string(result);
  remus::proto::Message message(this->MeshIOType,
                                    remus::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->ServerSocket);
}

}
}
