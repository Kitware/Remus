/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#include <meshserver/broker/Broker.h>

#include <boost/uuid/uuid.hpp>

#include <meshserver/internal/JobMessage.h>
#include <meshserver/internal/JobResponse.h>

#include <meshserver/internal/meshServerGlobals.h>
#include <meshserver/internal/messageHelper.h>
#include <meshserver/internal/zmqHelper.h>

#include <meshserver/broker/internal/JobQueue.h>


namespace meshserver{
namespace broker{

//------------------------------------------------------------------------------
Broker::Broker():
  UUIDGenerator(), //use default random number generator
  Jobs(new meshserver::broker::internal::JobQueue() ), //scoped ptr of JobQueue
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
      meshserver::internal::JobMessage message(this->JobQueries);

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
                                  const meshserver::internal::JobMessage& msg)
{
  //msg.dump(std::cout);
  //broker response is the general response message type
  //the client can than convert it to the expected type
  meshserver::internal::JobResponse response(clientAddress);
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
bool Broker::canMesh(const meshserver::internal::JobMessage& msg)
{
  //ToDo: add registration of mesh type
  //how is a generic worker going to register its type? static method?
  return true;
}

//------------------------------------------------------------------------------
meshserver::STATUS_TYPE Broker::meshStatus(const meshserver::internal::JobMessage& msg)
{
  boost::uuids::uuid id = meshserver::to_uuid(msg);
  if(this->Jobs->haveUUID(id))
    {
    return meshserver::QUEUED;
    }
  //ToDo: add tracking of mesh status from workers
  return meshserver::INVALID;
}

//------------------------------------------------------------------------------
std::string Broker::queueJob(const meshserver::internal::JobMessage& msg)
{
  //generate an UUID
  boost::uuids::uuid jobUUID = this->UUIDGenerator();
  //create a new job to place on the queue
  //This call will invalidate the msg as we are going to move the data
  //to another message to await sending to the worker
  this->Jobs->push(jobUUID,msg);

  //return the UUID
  return meshserver::to_string(jobUUID);
}

//------------------------------------------------------------------------------
std::string Broker::retrieveMesh(const meshserver::internal::JobMessage& msg)
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
}
