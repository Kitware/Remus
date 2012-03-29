/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/Broker.h>

#include <boost/uuid/uuid.hpp>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobStatus.h>
#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

#include <meshserver/broker/internal/uuidHelper.h>
#include <meshserver/broker/internal/JobQueue.h>


namespace meshserver{
namespace broker{

//------------------------------------------------------------------------------
Broker::Broker():
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new meshserver::broker::internal::JobQueue() ), //scoped ptr of JobQueue
  Context(1),
  JobQueries(this->Context,ZMQ_ROUTER),
  WorkerQueries(this->Context,ZMQ_ROUTER),
  Workers(this->Context,ZMQ_DEALER)
  {
  zmq::bindToSocket(JobQueries,meshserver::BROKER_CLIENT_PORT);
  zmq::bindToSocket(Workers,meshserver::BROKER_WORKER_PORT);
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

      //Note the contents of the message isn't valid
      //after the DetermineJobResponse call
      meshserver::JobMessage message(this->JobQueries);

      //NOTE: this will queue jobs if needed
      this->DetermineJobResponse(clientAddress,message);
      }
    else if (items [1].revents & ZMQ_POLLIN)
      {
      this->DetermineWorkerResponse();
      }
    else
      {
      //dispatch a job to a worker if we have any queued jobs and registered workers
      this->DispatchJob();
      }
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
  //how is a generic worker going to register its type? static method?
  return true;
}

//------------------------------------------------------------------------------
std::string Broker::meshStatus(const meshserver::JobMessage& msg)
{
  const boost::uuids::uuid id = meshserver::to_uuid(msg);
  const std::string sid = meshserver::to_string(id);

  meshserver::STATUS_TYPE type;

  if(this->QueuedJobs->haveUUID(id))
    {
    type = meshserver::QUEUED;
    }
  else
    {
    type = meshserver::INVALID;
    }
  const meshserver::common::JobStatus js(sid,type);
  return meshserver::to_string(js);
}

//------------------------------------------------------------------------------
std::string Broker::queueJob(const meshserver::JobMessage& msg)
{
  //generate an UUID
  boost::uuids::uuid jobUUID = this->UUIDGenerator();
  //create a new job to place on the queue
  //This call will invalidate the msg as we are going to move the data
  //to another message to await sending to the worker
  this->QueuedJobs->push(jobUUID,msg);

  //return the UUID
  return meshserver::to_string(jobUUID);
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
}
