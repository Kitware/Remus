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

#include <remus/server/Server.h>

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <remus/client/Job.h>
#include <remus/client/JobResult.h>
#include <remus/client/JobStatus.h>

#include <remus/worker/Job.h>
#include <remus/worker/JobResult.h>
#include <remus/worker/JobStatus.h>

#include <remus/common/Message.h>
#include <remus/common/Response.h>

#include <remus/common/remusGlobals.h>
#include <remus/common/zmqHelper.h>

#include <remus/server/detail/uuidHelper.h>
#include <remus/server/detail/ActiveJobs.h>
#include <remus/server/detail/JobQueue.h>
#include <remus/server/detail/WorkerPool.h>

#include <set>


//initialize the static instance variable in signal catcher in the class
//that inherits from it
remus::common::SignalCatcher* remus::common::SignalCatcher::Instance = NULL;

namespace remus{
namespace server{
namespace detail{

void make_terminateJob(remus::common::Response& response,
                       boost::uuids::uuid jobId)
{
  remus::worker::Job terminateJob(jobId,
                          remus::common::MeshIOType(),
                          remus::to_string(remus::TERMINATE_JOB_AND_WORKER));
  response.setServiceType(remus::TERMINATE_JOB_AND_WORKER);
  response.setData(remus::worker::to_string(terminateJob));
}

}
}
}

namespace remus{
namespace server{

//------------------------------------------------------------------------------
Server::Server():
  Context(1),
  ClientQueries(Context,ZMQ_ROUTER),
  WorkerQueries(Context,ZMQ_ROUTER),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  WorkerFactory(),
  PortInfo() //use default loopback ports
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(this->ClientQueries);
  this->PortInfo.bindWorker(this->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(const remus::server::WorkerFactory& factory):
  Context(1),
  ClientQueries(Context,ZMQ_ROUTER),
  WorkerQueries(Context,ZMQ_ROUTER),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  WorkerFactory(factory),
  PortInfo()
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(this->ClientQueries);
  this->PortInfo.bindWorker(this->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(remus::server::ServerPorts ports):
  Context(1),
  ClientQueries(Context,ZMQ_ROUTER),
  WorkerQueries(Context,ZMQ_ROUTER),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  WorkerFactory(),
  PortInfo(ports)
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(this->ClientQueries);
  this->PortInfo.bindWorker(this->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(remus::server::ServerPorts ports,
               const remus::server::WorkerFactory& factory):
  Context(1),
  ClientQueries(Context,ZMQ_ROUTER),
  WorkerQueries(Context,ZMQ_ROUTER),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  WorkerFactory(factory),
  PortInfo(ports)
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(this->ClientQueries);
  this->PortInfo.bindWorker(this->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::~Server()
{
  //the server is shutting down we need to terminate any workers that
  //are still running.
  this->TerminateAllWorkers();

  this->StopCatchingSignals();
}

//------------------------------------------------------------------------------
bool Server::startBrokering()
  {
  //start up signal catching before we start polling. We do this in the
  //startBrokering method since really the server isn't doing anything before
  //this point.
  this->StartCatchingSignals();

  zmq::pollitem_t items[2] = {
      { this->ClientQueries,  0, ZMQ_POLLIN, 0 },
      { this->WorkerQueries, 0, ZMQ_POLLIN, 0 } };

  //  Process messages from both sockets
  while (true)
    {
    zmq::poll(&items[0], 2, remus::HEARTBEAT_INTERVAL);
    // std::cout << "p" << std::endl;
    const boost::posix_time::ptime hbTime =
                          boost::posix_time::second_clock::local_time();
    if (items[0].revents & ZMQ_POLLIN)
      {
      //we need to strip the client address from the message
      zmq::socketIdentity clientIdentity = zmq::address_recv(this->ClientQueries);

      //Note the contents of the message isn't valid
      //after the DetermineJobQueryResponse call
      remus::common::Message message(this->ClientQueries);
      this->DetermineJobQueryResponse(clientIdentity,message); //NOTE: this will queue jobs

      // std::cout << "c" << std::endl;
      }
    if (items[1].revents & ZMQ_POLLIN)
      {
      //a worker is registering
      //we need to strip the worker address from the message
      zmq::socketIdentity workerIdentity = zmq::address_recv(this->WorkerQueries);

      //Note the contents of the message isn't valid
      //after the DetermineWorkerResponse call
      remus::common::Message message(this->WorkerQueries);
      this->DetermineWorkerResponse(workerIdentity,message);

      //refresh all jobs for a given worker with a new expiry time
      this->ActiveJobs->refreshJobs(workerIdentity);

      //refresh the worker if it is actuall in the pool instead of doing a job
      this->WorkerPool->refreshWorker(workerIdentity);

      // std::cout << "w" << std::endl;
      }

    //mark all jobs whose worker haven't sent a heartbeat in time
    //as a job that failed.
    this->ActiveJobs->markExpiredJobs(hbTime);

    //purge all pending workers with jobs that haven't sent a heartbeat
    this->WorkerPool->purgeDeadWorkers(hbTime);

    //see if we have a worker in the pool for the next job in the queue,
    //otherwise as the factory to generat a new worker to handle that job
    this->FindWorkerForQueuedJob();
    }

  //this should never be hit, but just incase lets make sure we close
  //down all workers.
  this->TerminateAllWorkers();

  this->StopCatchingSignals();

  return true;
  }

//------------------------------------------------------------------------------
void Server::DetermineJobQueryResponse(const zmq::socketIdentity& clientIdentity,
                                  const remus::common::Message& msg)
{
  //msg.dump(std::cout);
  //server response is the general response message type
  //the client can than convert it to the expected type
  remus::common::Response response(clientIdentity);
  if(!msg.isValid())
    {
    response.setServiceType(remus::INVALID_SERVICE);
    response.setData(remus::INVALID_MSG);
    response.send(this->ClientQueries);
    return; //no need to continue
    }
  response.setServiceType(msg.serviceType());

  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case remus::MAKE_MESH:
      response.setData(this->queueJob(msg));
      break;
    case remus::MESH_STATUS:
      response.setData(this->meshStatus(msg));
      break;
    case remus::CAN_MESH:
      response.setData(this->canMesh(msg));
      break;
    case remus::RETRIEVE_MESH:
      response.setData(this->retrieveMesh(msg));
      break;
    case remus::TERMINATE_JOB_AND_WORKER:
      response.setData(this->terminateJob(msg));
      break;
    default:
      response.setData(remus::INVALID_STATUS);
    }
  response.send(this->ClientQueries);
  return;
}

//------------------------------------------------------------------------------
bool Server::canMesh(const remus::common::Message& msg)
{
  //ToDo: add registration of mesh type
  //how is a generic worker going to register its type? static method?
  bool haveSupport = this->WorkerFactory.haveSupport(msg.MeshIOType());
  haveSupport = haveSupport || this->WorkerPool->haveWaitingWorker(msg.MeshIOType());
  return haveSupport;
}

//------------------------------------------------------------------------------
std::string Server::meshStatus(const remus::common::Message& msg)
{
  remus::client::Job job = remus::client::to_Job(msg.data());
  remus::client::JobStatus js(job.id(),INVALID_STATUS);
  if(this->QueuedJobs->haveUUID(job.id()))
    {
    js.Status = remus::QUEUED;
    }
  else if(this->ActiveJobs->haveUUID(job.id()))
    {
    js = this->ActiveJobs->status(job.id());
    }
  return remus::client::to_string(js);
}

//------------------------------------------------------------------------------
std::string Server::queueJob(const remus::common::Message& msg)
{
  if(this->canMesh(msg))
  {
    //generate an UUID
    const boost::uuids::uuid jobUUID = this->UUIDGenerator();
    //create a new job to place on the queue
    //This call will invalidate the msg as we are going to move the data
    //to another message to await sending to the worker
    this->QueuedJobs->addJob(jobUUID,msg);
    //return the UUID

    const remus::client::Job validJob(jobUUID,msg.MeshIOType());
    return remus::client::to_string(validJob);
  }
  return remus::INVALID_MSG;
}

//------------------------------------------------------------------------------
std::string Server::retrieveMesh(const remus::common::Message& msg)
{
  //go to the active jobs list and grab the mesh result if it exists
  remus::client::Job job = remus::client::to_Job(msg.data());

  remus::client::JobResult result(job.id());
  if( this->ActiveJobs->haveUUID(job.id()) &&
      this->ActiveJobs->haveResult(job.id()))
    {
    result = this->ActiveJobs->result(job.id());
    //for now we remove all references from this job being active
    this->ActiveJobs->remove(job.id());
    }
  //return an empty result
  return remus::client::to_string(result);
}

//------------------------------------------------------------------------------
std::string Server::terminateJob(const remus::common::Message& msg)
{

  remus::client::Job job = remus::client::to_Job(msg.data());

  bool removed = this->QueuedJobs->remove(job.id());
  if(!removed)
    {
    zmq::socketIdentity worker = this->ActiveJobs->workerAddress(job.id());
    removed = this->ActiveJobs->remove(job.id());

    //send an out of band message to the worker to kill itself
    //the trick is we are going to send a job to the worker which is constructed
    //to mean that it should die.
    if(removed && worker.size() > 0)
      {
      remus::common::Response response(worker);
      detail::make_terminateJob(response,job.id());
      response.send(this->WorkerQueries);
      }
    }

  remus::STATUS_TYPE status = (removed) ? remus::FAILED : remus::INVALID_STATUS;
  return remus::client::to_string(remus::client::JobStatus(job.id(),status));
}

//------------------------------------------------------------------------------
void Server::DetermineWorkerResponse(const zmq::socketIdentity &workerIdentity,
                                     const remus::common::Message& msg)
{
  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case remus::CAN_MESH:
      this->WorkerPool->addWorker(workerIdentity,msg.MeshIOType());
      break;
    case remus::MAKE_MESH:
      //the worker will block while it waits for a response.
      if(!this->WorkerPool->haveWorker(workerIdentity))
        {
        this->WorkerPool->addWorker(workerIdentity,msg.MeshIOType());
        }
      this->WorkerPool->readyForWork(workerIdentity);
      break;
    case remus::MESH_STATUS:
      //store the mesh status msg,  no response needed
      this->storeMeshStatus(msg);
      break;
    case remus::RETRIEVE_MESH:
      //we need to store the mesh result, no response needed
      this->storeMesh(msg);
      break;
    default:
      break;
    }
}

//------------------------------------------------------------------------------
void Server::storeMeshStatus(const remus::common::Message& msg)
{
  //the string in the data is actually a job status object
  remus::worker::JobStatus js = remus::worker::to_JobStatus(msg.data(),
                                                            msg.dataSize());
  this->ActiveJobs->updateStatus(js);
}

//------------------------------------------------------------------------------
void Server::storeMesh(const remus::common::Message& msg)
{
  remus::worker::JobResult jr = remus::worker::to_JobResult(msg.data(),
                                                            msg.dataSize());
  this->ActiveJobs->updateResult(jr);
}

//------------------------------------------------------------------------------
void Server::assignJobToWorker(const zmq::socketIdentity &workerIdentity,
                               const remus::worker::Job& job )
{
  this->ActiveJobs->add( workerIdentity, job.id() );

  remus::common::Response response(workerIdentity);
  response.setServiceType(remus::MAKE_MESH);

  std::string tmp = remus::worker::to_string(job);
  response.setData(tmp);
  response.send(this->WorkerQueries);
}

//see if we have a worker in the pool for the next job in the queue,
//otherwise ask the factory to generate a new worker to handle that job
//------------------------------------------------------------------------------
void Server::FindWorkerForQueuedJob()
{

  //We assume that a worker could possibly handle multiple jobs but all of the same type.
  //In order to prevent allocating more workers than needed we use a set instead of a vector.
  //This results in the server only creating one worker per job type.
  //This gives the new workers the opportunity of getting assigned multiple jobs.


  typedef std::set<remus::common::MeshIOType>::const_iterator it;
  this->WorkerFactory.updateWorkerCount();
  std::set<remus::common::MeshIOType> types;


  //find all the jobs that have been marked as waiting for a worker
  //and ask if we have a worker in the poll that can mesh that job
  types = this->QueuedJobs->waitingForWorkerTypes();
  for(it type = types.begin(); type != types.end(); ++type)
    {
    if(this->WorkerPool->haveWaitingWorker(*type))
      {
      //give this job to that worker
      this->assignJobToWorker(this->WorkerPool->takeWorker(*type),
                              this->QueuedJobs->takeJob(*type));
      }
    }


  //find all jobs that queued up and check if we can assign it to an item in
  //the worker pool
  types = this->QueuedJobs->queuedJobTypes();
  for(it type = types.begin(); type != types.end(); ++type)
    {
    if(this->WorkerPool->haveWaitingWorker(*type))
      {
      //give this job to that worker
      this->assignJobToWorker(this->WorkerPool->takeWorker(*type),
                              this->QueuedJobs->takeJob(*type));
      }
    }

  //now if we have room in our worker pool for more pending workers create some
  //make sure we ask the worker pool what its limit on number of pending
  //workers is before creating more. We have to requery to get the updated
  //job types since the worker pool might have taken some.
  types = this->QueuedJobs->queuedJobTypes();
  for(it type = types.begin(); type != types.end(); ++type)
    {
    //check if we have a waiting worker, if we don't than try
    //ask the factory to create a worker of that type.
    if(this->WorkerFactory.createWorker(*type,
                           WorkerFactory::KillOnFactoryDeletion))
      {
      this->QueuedJobs->workerDispatched(*type);
      }
    }
}


//We are crashing we need to terminate all workers
//------------------------------------------------------------------------------
void Server::SignalCaught( SignalCatcher::SignalType )
{
  //first step is to purge any workers that might have died since
  //the last time we polled. Because just maybe they also died
  //so no point in sending them a kill message now
  const boost::posix_time::ptime hbTime =
                          boost::posix_time::second_clock::local_time();
  this->ActiveJobs->markExpiredJobs(hbTime);
  this->WorkerPool->purgeDeadWorkers(hbTime);

  //Remove everything from the job queue so no new jobs start up.
  this->QueuedJobs->clear();

  this->TerminateAllWorkers();
}

//------------------------------------------------------------------------------
//terminate all workers that are doing jobs or waiting for jobs

void Server::TerminateAllWorkers( )
{

  //next we take workers from the worker pool and kill them all off
  std::set<zmq::socketIdentity> pendingWorkers =
                                              this->WorkerPool->livingWorkers();

  typedef std::set<zmq::socketIdentity>::const_iterator iterator;
  for(iterator i=pendingWorkers.begin(); i != pendingWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = this->UUIDGenerator();
    remus::common::Response response(*i);
    detail::make_terminateJob(response,jobId);
    response.send(this->WorkerQueries);
    }

  //lastly we will kill any still active worker
  std::set<zmq::socketIdentity> activeWorkers =
                                        this->ActiveJobs->activeWorkers();

  //only call terminate again on workers that are active
  for(iterator i=activeWorkers.begin(); i != activeWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = this->UUIDGenerator();
    remus::common::Response response(*i);
    detail::make_terminateJob(response,jobId);
    response.send(this->WorkerQueries);
    }

}


}
}
