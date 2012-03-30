/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_worker_h
#define __meshserver_worker_h

#include <zmq.hpp>
#include <string>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver
{
class Worker
{
public:
  //constuct a worker that can mesh a single type
  Worker(meshserver::MESH_TYPE mtype);

  //gets back a job from the broker
  //this will lock the worker as it will wait on a job message
  meshserver::common::JobDetails getJob();

  //update the status of the worker
  void updateStatus(const meshserver::common::JobStatus& info);

  //update the local cache of the mesh result.
  //this will be sent to the server when it requests our mesh result
  void returnMeshResults(const std::string& result);

private:
  //holds the type of mesh we support
  const meshserver::MESH_TYPE MeshType;

  zmq::context_t Context;

  //this socket is used to talk to broker
  zmq::socket_t Broker;
};

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype):
  MeshType(mtype),
  Context(1),
  Broker(this->Context,ZMQ_DEALER)
{
  zmq::connectToSocket(this->Broker,meshserver::BROKER_WORKER_PORT);
}

//-----------------------------------------------------------------------------
meshserver::common::JobDetails Worker::getJob()
{
  //send to the client that we are ready for a job.
  bool messageSent = false;
  meshserver::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);

  //start the polling before we send the message, so we can't miss the response
  zmq::pollitem_t item =  { this->Broker,  0, ZMQ_POLLIN, 0 };
  zmq::poll(&item,1,-1);
  canMesh.send(this->Broker);
  while(true)
    {
    if(item.revents & ZMQ_POLLIN)
      {
      //we have our message back
      meshserver::JobResponse response(this->Broker);

      //we need a better serialization techinque
      std::string msg = response.dataAs<std::string>();

      //see if we are given a job, or a request to poll again
      if(msg == meshserver::INVALID_MSG)
        {
        //send the request for a mesh again
        canMesh.send(this->Broker);
        }
      else
        {
        return meshserver::to_JobDetails(msg);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const meshserver::common::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = meshserver::to_string(info);
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::MESH_STATUS,
                                    msg.data(),msg.size());
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const std::string& result)
{
  //send a message that contains, the path to the resulting file
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::RETRIEVE_MESH,
                                    result.data(),result.size());

}


}
#endif
