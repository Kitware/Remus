/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#include "broker.h"

#include <boost/uuid/uuid.hpp>

#include "Common/meshServerGlobals.h"
#include "Common/zmqHelper.h"
#include "Common/messageHelper.h"
#include "Common/jobMessage.h"
#include "Common/jobResponse.h"

#include "jobQueue.h"

namespace meshserver
{

//------------------------------------------------------------------------------
Broker::Broker():
  UUIDGenerator(), //use default random number generator
  Jobs(new meshserver::JobQueue() ), //scoped ptr of JobQueue
  Context(1),
  JobQueries(this->Context,ZMQ_ROUTER),
  WorkerQueries(this->Context,ZMQ_ROUTER),
  Workers(this->Context,ZMQ_DEALER)
  {
  zmq::bindToSocket(JobQueries,meshserver::BROKER_CLIENT_PORT);
  //zmq::bindToSocket(Workers,meshserver::BROKER_WORKER_PORT);
  }

//------------------------------------------------------------------------------
Broker::~Broker()
{

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
      //we need to strip the client address from the message
      std::string clientAddress = zmq::s_recv(this->JobQueries);

      //Note:: the contents of the message isn't valid
      //after the DetermineJobResponse call
      meshserver::JobMessage message(this->JobQueries);

      //NOTE: this will queue jobs if needed
      this->DetermineJobResponse(clientAddress,message);
      }
    if (items [1].revents & ZMQ_POLLIN)
      {
      this->DetermineWorkerResponse();
      }
    //dispatch a job to a worker if we have any queued jobs
    this->DispatchJob();
    }
  return true;
  }

//------------------------------------------------------------------------------
void Broker::DetermineJobResponse(const std::string& clientAddress,
                                  const meshserver::JobMessage& msg)
{
  //msg.dump(std::cout);
  //broker response is the general response message type
  //the client can than convert it to the expected type
  meshserver::JobResponse response(clientAddress);
  if(!msg.isValid())
    {
    std::cout << "Sending invalid message" << std::endl;
    response.setData(meshserver::INVALID);
    response.send(this->JobQueries);
    return; //no need to continue
    }

  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case meshserver::MAKE_MESH:
      response.setData(this->queueJob(msg));
      break;
    case meshserver::MESH_STATUS:
      response.setData(this->meshStatus(msg));
      break;
    case meshserver::CAN_MESH:
      response.setData(this->canMesh(msg));
      break;
    case meshserver::RETRIEVE_MESH:
      response.setData(this->retrieveMesh(msg));
      break;
    default:
      std::cout << "Sending invalid message" << std::endl;
      response.setData(meshserver::INVALID);
    }
  response.send(this->JobQueries);
  return;
}

//------------------------------------------------------------------------------
bool Broker::canMesh(const meshserver::JobMessage& msg)
{
  //ToDo: add registration of mesh type
  return true;
}

//------------------------------------------------------------------------------
meshserver::STATUS_TYPE Broker::meshStatus(const meshserver::JobMessage& msg)
{
  boost::uuids::uuid id = meshserver::JobMessageToUUID(msg);
  if(this->Jobs->haveUUID(id))
    {
    return meshserver::QUEUED;
    }
  std::cout << "don't have uuid: " << id <<  std::endl;
  //ToDo: add tracking of mesh status from workers
  return meshserver::INVALID;
}

//------------------------------------------------------------------------------
std::string Broker::queueJob(const meshserver::JobMessage& msg)
{
  //generate an UUID
  boost::uuids::uuid jobUUID = this->UUIDGenerator();

  //create a new job to place on the queue
  //This call will invalidate the msg as we are going to move the data
  //to another message to await sending to the worker
  this->Jobs->push(jobUUID,msg);

  //return the UUID
  std::cout << "Generated UUID: " <<jobUUID <<std::endl;
  return meshserver::UUIDToString(jobUUID);
}

//------------------------------------------------------------------------------
std::string Broker::retrieveMesh(const meshserver::JobMessage& msg)
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

//------------------------------------------------------------------------------
void Broker::DispatchJob()
{
  //no-op for now
}


}
