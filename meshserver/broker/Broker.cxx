/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/Broker.h>

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

#include <meshserver/broker/internal/uuidHelper.h>
#include <meshserver/broker/internal/ActiveJobs.h>
#include <meshserver/broker/internal/JobQueue.h>
#include <meshserver/broker/internal/WorkerFactory.h>
#include <meshserver/broker/internal/WorkerPool.h>


namespace meshserver{
namespace broker{

//------------------------------------------------------------------------------
Broker::Broker():
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new meshserver::broker::internal::JobQueue() ),
  WorkerPool(new meshserver::broker::internal::WorkerPool() ),
  ActiveJobs(new meshserver::broker::internal::ActiveJobs () ),
  WorkerFactory(new meshserver::broker::internal::WorkerFactory() ),
  Context(1),
  JobQueries(this->Context,ZMQ_ROUTER),
  WorkerQueries(this->Context,ZMQ_ROUTER)
  {
  zmq::bindToSocket(JobQueries,meshserver::BROKER_CLIENT_PORT);
  zmq::bindToSocket(WorkerQueries,meshserver::BROKER_WORKER_PORT);
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
      { this->WorkerQueries, 0, ZMQ_POLLIN, 0 } };

  //  Process messages from both sockets
  while (true)
    {
    zmq::poll (&items[0], 2, meshserver::HEARTBEAT_INTERVAL);
    boost::posix_time::ptime hbTime = boost::posix_time::second_clock::local_time();
    if (items[0].revents & ZMQ_POLLIN)
      {
      //we need to strip the client address from the message
      std::string clientAddress = zmq::s_recv(this->JobQueries);

      //Note the contents of the message isn't valid
      //after the DetermineJobQueryResponse call
      meshserver::JobMessage message(this->JobQueries);
      this->DetermineJobQueryResponse(clientAddress,message); //NOTE: this will queue jobs
      }
    else if (items[1].revents & ZMQ_POLLIN)
      {
      //a worker is registering
      //we need to strip the worker address from the message
      std::string workerAddress = zmq::s_recv(this->WorkerQueries);

      //Note the contents of the message isn't valid
      //after the DetermineWorkerResponse call
      meshserver::JobMessage message(this->WorkerQueries);
      this->DetermineWorkerResponse(workerAddress,message);

      //refresh all jobs for a given worker with a new expiry time
      this->ActiveJobs->refreshJobs(workerAddress);

      //refresh the worker if it is actuall in the pool instead of doing a job
      this->WorkerPool->refreshWorker(workerAddress);
      }

    //mark all jobs whose worker haven't sent a heartbeat in time
    //as a job that failed.
    this->ActiveJobs->markFailedJobs(hbTime);

    //purge all pending workers with jobs that haven't sent a heartbeat
    this->WorkerPool->purgeDeadWorkers(hbTime);

    //see if we have a worker in the pool for the next job in the queue,
    //otherwise as the factory to generat a new worker to handle that job
    this->FindWorkerForQueuedJob();
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
      response.setData(meshserver::INVALID_STATUS);
    }
  response.send(this->JobQueries);
  return;
}

//------------------------------------------------------------------------------
bool Broker::canMesh(const meshserver::JobMessage& msg)
{
  //ToDo: add registration of mesh type
  //how is a generic worker going to register its type? static method?
  return this->WorkerFactory->haveSupport(msg.meshType());
}

//------------------------------------------------------------------------------
std::string Broker::meshStatus(const meshserver::JobMessage& msg)
{
  const boost::uuids::uuid id = meshserver::to_uuid(msg);

  meshserver::common::JobStatus js(id,INVALID_STATUS);
  if(this->QueuedJobs->haveUUID(id))
    {
    js.Status = meshserver::QUEUED;
    }
  else if(this->ActiveJobs->haveUUID(id))
    {
    js = this->ActiveJobs->status(id);
    }
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

  meshserver::common::JobResult result(id);
  if(this->ActiveJobs->haveUUID(id) && this->ActiveJobs->haveResult(id))
    {
    result = this->ActiveJobs->result(id);
    }

  //for now we remove all references from this job being active
  this->ActiveJobs->remove(id);

  return meshserver::to_string(result);
}

//------------------------------------------------------------------------------
void Broker::DetermineWorkerResponse(const std::string &workAddress,
                                     const meshserver::JobMessage& msg)
{
  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case meshserver::CAN_MESH:
      this->WorkerPool->addWorker(workAddress,msg.meshType());
      break;
    case meshserver::MAKE_MESH:
    //the worker will block while it waits for a response.
      if(this->haveJobForWorker(msg))
        {
        //we currently have a job for the worker, give it back now
        this->assignJobToWorker(workAddress);
        }
      else
        {
        //we don't currently have a job, add the worker to a pool
        //and check each event loop time if we have a job for that worker
        if(!this->WorkerPool->haveWorker(workAddress))
          {
          this->WorkerPool->addWorker(workAddress,msg.meshType());
          }
        this->WorkerPool->readyForWork(workAddress);
        }

      break;
    case meshserver::MESH_STATUS:
      //store the mesh status msg,  no response needed
      this->storeMeshStatus(msg);
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
  this->ActiveJobs->updateStatus(js);
}

//------------------------------------------------------------------------------
void Broker::storeMesh(const meshserver::JobMessage& msg)
{
  meshserver::common::JobResult jr = meshserver::to_JobResult(msg.data(),msg.dataSize());
  this->ActiveJobs->updateResult(jr);
}

//------------------------------------------------------------------------------
bool Broker::haveJobForWorker(const meshserver::JobMessage& msg) const
{
  //lets see if the msg mesh type and the queue match up
  return (this->QueuedJobs->size() > 0 && msg.meshType() == this->QueuedJobs->front().meshType());
}

//------------------------------------------------------------------------------
void Broker::assignJobToWorker(const std::string &workAddress)
{
  meshserver::common::JobDetails jd = this->QueuedJobs->pop();
  this->ActiveJobs->add( workAddress, jd.JobId );

  meshserver::JobResponse response(workAddress);
  response.setData(meshserver::to_string(jd));
  response.send(this->WorkerQueries);
}

//see if we have a worker in the pool for the next job in the queue,
//otherwise as the factory to generat a new worker to handle that job
//------------------------------------------------------------------------------
void Broker::FindWorkerForQueuedJob()
{
  //if we have something in the job queue ask the factory to generate a worker
  //for that job

  //this has a bug that we can launch multiple workers for a same job before
  //a single worker reports back. We need to rethink this entire framework.
  //most likely change the QueuedJobs to a pool that has queued and pending status

  if(this->QueuedJobs->size() == 0)
    {
    return;
    }

  meshserver::MESH_TYPE type = this->QueuedJobs->front().meshType();
  if(this->WorkerPool->haveWaitingWorker(type))
    {
    //give this job to that worker
    this->assignJobToWorker(this->WorkerPool->takeWorker(type));
    }
  else
    {
    this->WorkerFactory->createWorker(type);
    }
}

}
}
