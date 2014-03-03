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

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <remus/proto/Job.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/JobRequirements.h>
#include <remus/proto/Message.h>
#include <remus/proto/Response.h>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/proto/zmqHelper.h>

#include <remus/worker/Job.h>

#include <remus/common/remusGlobals.h>

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

//------------------------------------------------------------------------------
void make_terminateWorker(remus::proto::Response& response,
                          boost::uuids::uuid jobId)
{
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());
  response.setServiceType(remus::TERMINATE_WORKER);
  response.setData(remus::worker::to_string(terminateJob));
}

//------------------------------------------------------------------------------
void make_terminateJob(remus::proto::Response& response,
                       boost::uuids::uuid jobId)
{
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());
  response.setServiceType(remus::TERMINATE_JOB);
  response.setData(remus::worker::to_string(terminateJob));
}

//------------------------------------------------------------------------------
struct ThreadManagement
{
  //----------------------------------------------------------------------------
  ThreadManagement():
    BrokerThread( new boost::thread() ),
    BrokeringStatus(),
    BrokerIsRunning(false)
  {
  }

  //----------------------------------------------------------------------------
  ~ThreadManagement()
  {
  this->stop();
  }

  //----------------------------------------------------------------------------
  bool isBrokering()
  {
  boost::lock_guard<boost::mutex> lock(this->BrokeringStatus);
  return this->BrokerIsRunning;
  }

  //----------------------------------------------------------------------------
  bool start(remus::server::Server* server,
             remus::server::Server::SignalHandling sigHandleState)
  {
  bool launchThread = false;

    //lock the construction of thread as a critical section
    {
    boost::unique_lock<boost::mutex> lock(this->BrokeringStatus);
    launchThread = !this->BrokerIsRunning;
    if(launchThread)
      {
      boost::scoped_ptr<boost::thread> bthread(
        new  boost::thread(&Server::brokering, server, sigHandleState) );
      this->BrokerThread.swap(bthread);
      }
    }

  //do this outside the previous critical section so that we properly
  //tell other threads that the thread has been launched. We don't
  //want to cause a recursive lock in the same thread to happen
  if(launchThread)
    {
    this->setIsBrokering(true);
    }

  return this->isBrokering();
  }

  //----------------------------------------------------------------------------
  void stop()
  {
  this->setIsBrokering(false);
  this->BrokerThread->join();
  }

  //----------------------------------------------------------------------------
  void waitForThreadToStart()
  {
  boost::unique_lock<boost::mutex> lock(this->BrokeringStatus);
  while(!this->BrokerIsRunning)
    {
    BrokerStatusChanged.wait(lock);
    }
  }

  //----------------------------------------------------------------------------
  void waitForThreadToFinish()
  {
  //first we wait for the Broker to start up
  this->waitForThreadToStart();

  //now we wait for the broker to finish
  this->BrokerThread->join();
  }

private:
  boost::scoped_ptr<boost::thread> BrokerThread;

  boost::mutex BrokeringStatus;
  boost::condition_variable BrokerStatusChanged;
  bool BrokerIsRunning;

  //----------------------------------------------------------------------------
  void setIsBrokering(bool t)
  {
    {
    boost::unique_lock<boost::mutex> lock(this->BrokeringStatus);
    this->BrokerIsRunning = t;
    }
  this->BrokerStatusChanged.notify_all();
  }
};

//------------------------------------------------------------------------------
struct ZmqManagement
{
  zmq::context_t Context;
  zmq::socket_t ClientQueries;
  zmq::socket_t WorkerQueries;

  //----------------------------------------------------------------------------
  ZmqManagement():
    Context(2),
    ClientQueries(Context,ZMQ_ROUTER),
    WorkerQueries(Context,ZMQ_ROUTER)
  {}
};


}
}
}

namespace remus{
namespace server{

//------------------------------------------------------------------------------
Server::Server():
  Zmq(new detail::ZmqManagement() ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  Thread(new detail::ThreadManagement() ),
  WorkerFactory(),
  PortInfo() //use default loopback ports
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(const remus::server::WorkerFactory& factory):
  Zmq(new detail::ZmqManagement() ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  Thread(new detail::ThreadManagement() ),
  WorkerFactory(factory),
  PortInfo()
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(remus::server::ServerPorts ports):
  Zmq(new detail::ZmqManagement() ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  Thread(new detail::ThreadManagement() ),
  WorkerFactory(),
  PortInfo(ports)
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(remus::server::ServerPorts ports,
               const remus::server::WorkerFactory& factory):
  Zmq(new detail::ZmqManagement() ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs(new remus::server::detail::JobQueue() ),
  WorkerPool(new remus::server::detail::WorkerPool() ),
  ActiveJobs(new remus::server::detail::ActiveJobs () ),
  Thread(new detail::ThreadManagement() ),
  WorkerFactory(factory),
  PortInfo(ports)
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory.addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::~Server()
{

}

//------------------------------------------------------------------------------
bool Server::brokering(Server::SignalHandling sh)
  {
  //start up signal catching before we start polling. We do this in the
  //startBrokering method since really the server isn't doing anything before
  //this point.
  switch (sh)
    {
    case CAPTURE:
        this->StartCatchingSignals();
        break;
    case NONE:
        //do nothing
        break;
    }

  zmq::pollitem_t items[2] = {
      { this->Zmq->ClientQueries,  0, ZMQ_POLLIN, 0 },
      { this->Zmq->WorkerQueries, 0, ZMQ_POLLIN, 0 } };

  //  Process messages from both sockets
  while (Thread->isBrokering())
    {
    zmq::poll(&items[0], 2, remus::HEARTBEAT_INTERVAL);
    const boost::posix_time::ptime hbTime =
                            boost::posix_time::second_clock::local_time();
    if (items[0].revents & ZMQ_POLLIN)
      {
      //we need to strip the client address from the message
      zmq::SocketIdentity clientIdentity = zmq::address_recv(this->Zmq->ClientQueries);

      //Note the contents of the message isn't valid
      //after the DetermineJobQueryResponse call
      remus::proto::Message message(&this->Zmq->ClientQueries);
      this->DetermineJobQueryResponse(clientIdentity,message); //NOTE: this will queue jobs
      }
    if (items[1].revents & ZMQ_POLLIN)
      {
      //a worker is registering
      //we need to strip the worker address from the message
      zmq::SocketIdentity workerIdentity = zmq::address_recv(this->Zmq->WorkerQueries);

      //Note the contents of the message isn't valid
      //after the DetermineWorkerResponse call
      remus::proto::Message message(&this->Zmq->WorkerQueries);
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

  //this should only happen with interrupted threads be hit; lets make sure we close
  //down all workers.
  this->TerminateAllWorkers();

  this->StopCatchingSignals();

  return true;
  }

//------------------------------------------------------------------------------
bool Server::startBrokering(SignalHandling sh)
{
  return this->Thread->start(this, sh);
}

//------------------------------------------------------------------------------
void Server::stopBrokering()
{
  this->Thread->stop();
}

//------------------------------------------------------------------------------
void Server::waitForBrokeringToFinish()
{
  this->Thread->waitForThreadToFinish();
}

//------------------------------------------------------------------------------
void Server::waitForBrokeringToStart()
{
  this->Thread->waitForThreadToStart();
}

//------------------------------------------------------------------------------
void Server::DetermineJobQueryResponse(const zmq::SocketIdentity& clientIdentity,
                                  const remus::proto::Message& msg)
{
  //msg.dump(std::cout);
  //server response is the general response message type
  //the client can than convert it to the expected type
  remus::proto::Response response(clientIdentity);
  if(!msg.isValid())
    {
    response.setServiceType(remus::INVALID_SERVICE);
    response.setData( remus::INVALID_MSG );
    response.send(&this->Zmq->ClientQueries);
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
    case remus::CAN_MESH_REQUIREMENTS:
      response.setData(this->canMeshRequirements(msg));
      break;
    case remus::MESH_REQUIREMENTS:
      response.setData(this->meshRequirements(msg));
      break;
    case remus::RETRIEVE_MESH:
      response.setData(this->retrieveMesh(msg));
      break;
    case remus::TERMINATE_JOB:
      response.setData(this->terminateJob(msg));
      break;
    default:
      response.setData( remus::to_string(remus::INVALID_STATUS) );
    }
  response.send(&this->Zmq->ClientQueries);
  return;
}

//------------------------------------------------------------------------------
std::string Server::canMesh(const remus::proto::Message& msg)
{
  //we state that the factory can support a mesh type by having a worker
  //registered to it that supports the mesh type.
  bool workerSupport =
    (this->WorkerFactory.workerRequirements(msg.MeshIOType()).size() > 0) &&
    (this->WorkerFactory.maxWorkerCount() > 0);

  //Query the worker pool to get the set of requirements for waiting
  //workers that support the given mesh type info
  bool poolSupport =
    (this->WorkerPool->waitingWorkerRequirements(msg.MeshIOType()).size() > 0);

  std::ostringstream buffer;
  buffer << (workerSupport || poolSupport);
  return buffer.str();
}

//------------------------------------------------------------------------------
std::string Server::canMeshRequirements(const remus::proto::Message& msg)
{
  //we state that the factory can support a mesh type by having a worker
  //registered to it that supports the mesh type.
  remus::proto::JobRequirements reqs =
                              remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
  bool workerSupport = this->WorkerFactory.haveSupport(reqs) &&
                      (this->WorkerFactory.maxWorkerCount() > 0);

  //Query the worker pool to get the set of requirements for waiting
  //workers that support the given mesh type info
  bool poolSupport = this->WorkerPool->haveWaitingWorker(reqs);

  std::ostringstream buffer;
  buffer << (workerSupport || poolSupport);
  return buffer.str();
}

//------------------------------------------------------------------------------
std::string Server::meshRequirements(const remus::proto::Message& msg)
{
  //we state that the factory can support a mesh type by having a worker
  //registered to it that supports the mesh type.
  remus::proto::JobRequirementsSet reqSet(
            this->WorkerFactory.workerRequirements(msg.MeshIOType()));

  //Query the worker pool to get the set of requirements for waiting
  //workers that support the given mesh type info
  remus::proto::JobRequirementsSet poolSet =
            this->WorkerPool->waitingWorkerRequirements(msg.MeshIOType());

  //combine the two sets to get all the valid requirements
  reqSet.insert(poolSet.begin(),poolSet.end());

  return remus::proto::to_string(reqSet);
}

//------------------------------------------------------------------------------
std::string Server::meshStatus(const remus::proto::Message& msg)
{
  remus::proto::Job job = remus::proto::to_Job(msg.data(),msg.dataSize());
  remus::proto::JobStatus js(job.id(),remus::INVALID_STATUS);
  if(this->QueuedJobs->haveUUID(job.id()))
    {
    js = remus::proto::JobStatus(job.id(),remus::QUEUED);
    }
  else if(this->ActiveJobs->haveUUID(job.id()))
    {
    js = this->ActiveJobs->status(job.id());
    }
  return remus::proto::to_string(js);
}

//------------------------------------------------------------------------------
std::string Server::queueJob(const remus::proto::Message& msg)
{
  //generate an UUID
  const boost::uuids::uuid jobUUID = this->UUIDGenerator();

  //create a new job to place on the queue
  const remus::proto::JobSubmission submission =
                  remus::proto::to_JobSubmission(msg.data(),msg.dataSize());

  this->QueuedJobs->addJob(jobUUID,submission);
  //return the UUID

  const remus::proto::Job validJob(jobUUID,msg.MeshIOType());
  return remus::proto::to_string(validJob);
}

//------------------------------------------------------------------------------
std::string Server::retrieveMesh(const remus::proto::Message& msg)
{
  //go to the active jobs list and grab the mesh result if it exists
  remus::proto::Job job = remus::proto::to_Job(msg.data(),msg.dataSize());

  remus::proto::JobResult result(job.id());
  if( this->ActiveJobs->haveUUID(job.id()) &&
      this->ActiveJobs->haveResult(job.id()))
    {
    result = this->ActiveJobs->result(job.id());
    //for now we remove all references from this job being active
    this->ActiveJobs->remove(job.id());
    }
  //return an empty result
  return remus::proto::to_string(result);
}

//------------------------------------------------------------------------------
std::string Server::terminateJob(const remus::proto::Message& msg)
{

  remus::proto::Job job = remus::proto::to_Job(msg.data(),msg.dataSize());

  bool removed = this->QueuedJobs->remove(job.id());
  if(!removed)
    {
    zmq::SocketIdentity worker = this->ActiveJobs->workerAddress(job.id());
    removed = this->ActiveJobs->remove(job.id());

    //send an out of band message to the worker to kill itself
    //the trick is we are going to send a job to the worker which is constructed
    //to mean that it should die.
    if(removed && worker.size() > 0)
      {
      remus::proto::Response response(worker);
      detail::make_terminateJob(response,job.id());
      response.send(&this->Zmq->WorkerQueries);
      }
    }

  remus::STATUS_TYPE status = (removed) ? remus::FAILED : remus::INVALID_STATUS;
  return remus::proto::to_string(remus::proto::JobStatus(job.id(),status));
}

//------------------------------------------------------------------------------
void Server::DetermineWorkerResponse(const zmq::SocketIdentity &workerIdentity,
                                     const remus::proto::Message& msg)
{
  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case remus::CAN_MESH:
      {
      const remus::proto::JobRequirements reqs =
            remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
      this->WorkerPool->addWorker(workerIdentity,reqs);
      }
      break;
    case remus::MAKE_MESH:
      {
      const remus::proto::JobRequirements reqs =
            remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
      if(!this->WorkerPool->haveWorker(workerIdentity))
        {
        this->WorkerPool->addWorker(workerIdentity,reqs);
        }
      this->WorkerPool->readyForWork(workerIdentity);
      }
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
void Server::storeMeshStatus(const remus::proto::Message& msg)
{
  //the string in the data is actually a job status object
  remus::proto::JobStatus js = remus::proto::to_JobStatus(msg.data(),
                                                          msg.dataSize());
  this->ActiveJobs->updateStatus(js);
}

//------------------------------------------------------------------------------
void Server::storeMesh(const remus::proto::Message& msg)
{
  remus::proto::JobResult jr = remus::proto::to_JobResult(msg.data(),
                                                            msg.dataSize());
  this->ActiveJobs->updateResult(jr);
}

//------------------------------------------------------------------------------
void Server::assignJobToWorker(const zmq::SocketIdentity &workerIdentity,
                               const remus::worker::Job& job )
{
  this->ActiveJobs->add( workerIdentity, job.id() );

  remus::proto::Response response(workerIdentity);
  response.setServiceType(remus::MAKE_MESH);

  std::string tmp = remus::worker::to_string(job);
  response.setData(tmp);
  response.send(&this->Zmq->WorkerQueries);
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
  this->WorkerFactory.updateWorkerCount();


  typedef remus::proto::JobRequirementsSet::const_iterator it;
  remus::proto::JobRequirementsSet types;

  //find all the jobs that have been marked as waiting for a worker
  //and ask if we have a worker in the poll that can mesh that job
  types = this->QueuedJobs->waitingJobRequirements();
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
  types = this->QueuedJobs->queuedJobRequirements();
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
  types = this->QueuedJobs->queuedJobRequirements();
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
  this->Thread->stop();

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
  std::set<zmq::SocketIdentity> pendingWorkers =
                                              this->WorkerPool->livingWorkers();

  typedef std::set<zmq::SocketIdentity>::const_iterator iterator;
  for(iterator i=pendingWorkers.begin(); i != pendingWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = this->UUIDGenerator();

    remus::proto::Response response(*i);
    detail::make_terminateWorker(response,jobId);

    response.send(&this->Zmq->WorkerQueries);
    }

  //lastly we will kill any still active worker
  std::set<zmq::SocketIdentity> activeWorkers =
                                        this->ActiveJobs->activeWorkers();

  //only call terminate again on workers that are active
  for(iterator i=activeWorkers.begin(); i != activeWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = this->UUIDGenerator();

    remus::proto::Response response(*i);
    detail::make_terminateWorker(response,jobId);

    response.send(&this->Zmq->WorkerQueries);
    }

}


}
}
