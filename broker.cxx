/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#include "broker.h"

#include "Common/meshServerGlobals.h"
#include "Common/zmqHelper.h"
#include "Common/job.h"
#include "Common/Response.h"

namespace meshserver
{

//------------------------------------------------------------------------------
Broker::Broker():
  Context(1),
  JobQueries(this->Context,ZMQ_ROUTER),
  WorkerQueries(this->Context,ZMQ_ROUTER),
  Workers(this->Context,ZMQ_DEALER)
  {
  zmq::bindToSocket(JobQueries,meshserver::BROKER_CLIENT_PORT);
  //zmq::bindToSocket(Workers,meshserver::BROKER_WORKER_PORT);
  }

//------------------------------------------------------------------------------
bool Broker::startBrokering()
  {
  zmq::pollitem_t items[2] = {
      { this->JobQueries,  0, ZMQ_POLLIN, 0 },
      { this->Workers, 0, ZMQ_POLLIN, 0 } };

  //  Process messages from both sockets
  while (true)
    {
    zmq::poll (&items[0], 2, -1);
    if (items [0].revents & ZMQ_POLLIN)
      {
      meshserver::JobMessage message(this->JobQueries);
      this->DetermineJobResponse(&message);
      }
    if (items [1].revents & ZMQ_POLLIN)
      {
      this->DetermineWorkerResponse();
      }
    }
  return true;
  }

//------------------------------------------------------------------------------
void Broker::DetermineJobResponse(JobMessage *jmsg)
{
  //broker response is the general response message type
  //the client can than convert it to the expected type
  meshserver::JobResponse response;
  if(!jmsg->isValid())
    {
    response.markInvalid();
    response.send(this->JobQueries);
    return; //no need to continue
    }

  //we have a valid job, determine what to do with it
  switch(jmsg.serviceType())
    {
    case meshserver::MAKE_MESH:
      response.setData(this->queueJob(jmsg));
      break;
    case meshserver::MESH_STATUS:
      response.setData(this->meshStatus(jmsg));
      break;
    case meshserver::CAN_MESH:
      response.setData(this->canMesh(jmsg));
      break;
    case meshserver::RETRIEVE_MESH:
      response.setData(this->retrieveMesh(jmsg));
      break;
    default:
      response.markInvalid();
    }
  response.send(this->JobQueries);
  return;
}

//------------------------------------------------------------------------------
bool canMesh(meshserver::JobMessage* msg)
{
  //ToDo: add registration of mesh type
  return true;
}

//------------------------------------------------------------------------------
meshserver::STATUS_TYPE meshStatus(meshserver::JobMessage* msg)
{
  //ToDo: add tracking of mesh status from workers
  return meshserver::INVALID;
}

//------------------------------------------------------------------------------
std::string queueJob(meshserver::JobMessage* msg)
{
  //ToDo: The broker needs to generate a UUID for the job that it tracks by
  // use boost uuid to generate the ids that we track by
  return std::string("0123456789abcdef0123456789abcdef");
}

//------------------------------------------------------------------------------
std::string retrieveMesh(meshserver::JobMessage* msg)
{
  //ToDo: actually have a way for a worker to return the mesh.
  //I think we need to design a system where the broker holds onto meshes
  //after they are complete, and frees up workers
  return std::string("NO_PATH");
}


//------------------------------------------------------------------------------
void Broker::DetermineWorkerResponse()
{
  //no-op for now
}

}
