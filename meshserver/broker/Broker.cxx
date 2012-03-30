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
#include <meshserver/common/JobResult.h>
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
      //after the DetermineJobQueryResponse call
      meshserver::JobMessage message(this->JobQueries);
      this->DetermineJobQueryResponse(clientAddress,message); //NOTE: this will queue jobs
      }
    else if (items [1].revents & ZMQ_POLLIN)
      {
      //a worker is registering
      //we need to strip the worker address from the message
      std::string workerAddress = zmq::s_recv(this->Workers);

      //Note the contents of the message isn't valid
      //after the DetermineWorkerResponse call
      meshserver::JobMessage message(this->Workers);
      this->DetermineWorkerResponse(workerAddress,message);
      }

    //ask the factory to generate a worker
    //that is of the same type as the first queued job
    this->GenerateWorkerForQueuedJob();
    }
  return true;
  }

//------------------------------------------------------------------------------
void Broker::DetermineJobQueryResponse(const std::string& clientAddress,
                                  const meshserver::JobMessage& msg)
{
  //msg.dump(std::cout);
  //broker response is the general response message type
  //the client can than convert it to the expected type
  meshserver::JobResponse response(clientAddress);
  if(!msg.isValid())
    {
    response.setData(meshserver::INVALID_MSG);
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
  return this->WorkerFactory.haveSupport(msg.meshType());
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
  else if(this->ActiveJobs->haveUUID(id))
    {
    type = this->ActiveJobs->status(id);
    }
  else
    {
    type = meshserver::INVALID_STATUS;
    }
  const meshserver::common::JobStatus js(sid,type);
  return meshserver::to_string(js);
}

//------------------------------------------------------------------------------
std::string Broker::queueJob(const meshserver::JobMessage& msg)
{
  if(this->canMesh(msg))
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
  return meshserver::INVALID_MSG;
}

//------------------------------------------------------------------------------
std::string Broker::retrieveMesh(const meshserver::JobMessage& msg)
{
  //go to the active jobs list and grab the mesh result if it exists
  const boost::uuids::uuid id = meshserver::to_uuid(msg);
  const std::string sid = meshserver::to_string(id);

  meshserver::common::JobResults result(sid);
  if(this->ActiveJobs->haveUUID(id) && this->ActiveJobs->jobFinished(id))
    {
    result = this->ActiveJobs->results(id);
    }
  return result;
}

//------------------------------------------------------------------------------
void Broker::DetermineWorkerResponse(const std::string &workAddress,
                                     const meshserver::JobMessage& msg)
{
  meshserver::JobResponse response(workAddress);

  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case meshserver::MESH_STATUS:
      //store the mesh status msg, have to respond
      response.setData(this->storeMeshStatus(msg));
      response.send(this->JobQueries);
      break;
    case meshserver::CAN_MESH:
      //retrieve a job for the worker, no response needed
      this->getJob(msg);
      break;
    case meshserver::RETRIEVE_MESH:
      //we need to store the mesh result, no response needed
      this->storeMesh(msg);
      break;
    default:
      break;
    }
}

//------------------------------------------------------------------------------
void Broker::storeMeshStatus(const meshserver::JobMessage& msg)
{
  //the string in the data is actualy a job status object
  meshserver::common::JobStatus js = meshserver::to_JobStatus(msg.data(),msg.dataSize());
  this->ActiveJobs.updateStatus(js);
}

//------------------------------------------------------------------------------
void Broker::storeMesh(const meshserver::JobMessage& msg)
{
  meshserver::common::JobResults jr = meshserver::to_JobResults(msg.data(),msg.dataSize());
  this->ActiveJobs.updateResults(jr);
}

//------------------------------------------------------------------------------
std::string Broker::getJob(const meshserver::JobMessage& msg)
{
  //lets see if the msg mesh type and the queue match up
  if(msg.meshType() == this->QueuedJobs.front().meshType())
    {
      meshserver::common::JobDetails jd = this->QueuedJobs.pop();
      return meshserver::to_string(jd);
    }
  return meshserver::INVALID_MSG;
}

//------------------------------------------------------------------------------
void Broker::GenerateWorkerForQueuedJob()
{
  //if we have something in the job queue ask the factory to generate a worker
  //for that job
  if(this->QueuedJobs.size() > 0 )
    {
    this->WorkerFactory.createWorker(this->QueuedJobs.front().meshType());
    }
}

}
}
