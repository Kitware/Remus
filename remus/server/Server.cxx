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

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/thread.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <boost/make_shared.hpp>
#include <boost/thread/locks.hpp>
#include <boost/uuid/uuid.hpp>

#include <remus/proto/Job.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/JobRequirements.h>
#include <remus/proto/Message.h>
#include <remus/proto/Response.h>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/proto/zmqHelper.h>

#include <remus/worker/Job.h>

#include <remus/common/PollingMonitor.h>

#include <remus/server/detail/uuidHelper.h>
#include <remus/server/detail/ActiveJobs.h>
#include <remus/server/detail/EventPublisher.h>
#include <remus/server/detail/JobQueue.h>
#include <remus/server/detail/SocketMonitor.h>
#include <remus/server/detail/WorkerPool.h>
#include <remus/server/WorkerFactory.h>

#include <set>
#include <ctime>

//initialize the static instance variable in signal catcher in the class
//that inherits from it
remus::common::SignalCatcher* remus::common::SignalCatcher::Instance = NULL;

namespace remus{
namespace server{
namespace detail{

//------------------------------------------------------------------------------
void send_terminateWorker(boost::uuids::uuid jobId,
                          zmq::socket_t& socket,
                          const zmq::SocketIdentity& workerId)
{
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());

  remus::proto::send_NonBlockingResponse(remus::TERMINATE_WORKER,
                                         remus::worker::to_string(terminateJob),
                                         &socket,
                                         workerId);
}

//------------------------------------------------------------------------------
void send_terminateJob(boost::uuids::uuid jobId,
                          zmq::socket_t& socket,
                          const zmq::SocketIdentity& workerId)
{
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());

  remus::proto::send_NonBlockingResponse(remus::TERMINATE_JOB,
                                         remus::worker::to_string(terminateJob),
                                         &socket,
                                         workerId);
}

//------------------------------------------------------------------------------
struct UUIDManagement
{
  //----------------------------------------------------------------------------
  UUIDManagement():
  twister( static_cast<unsigned int>(std::time(0)) ),
  generator(&this->twister)
  {

  }

  inline boost::uuids::uuid operator()()
  {
    return this->generator();
  }

private:
  boost::mt19937 twister;
  boost::uuids::basic_random_generator<boost::mt19937> generator;
};

//------------------------------------------------------------------------------
struct ThreadManagement
{
  //----------------------------------------------------------------------------
  ThreadManagement():
    BrokerThread( new boost::thread() ),
    BrokeringStatus(),
    BrokerStatusChanged(),
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
    boost::lock_guard<boost::mutex> lock(this->BrokeringStatus);
    launchThread = !this->BrokerIsRunning;
    if(launchThread)
      {
      boost::scoped_ptr<boost::thread> bthread(
        new  boost::thread(&Server::Brokering, server, sigHandleState) );
      this->BrokerThread.swap(bthread);
      }
    }

  //do this outside the previous critical section so that we properly
  //tell other threads that the thread has been launched. We don't
  //want to cause a recursive lock in the same thread to happen
  this->waitForThreadToStart();
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
  if(this->BrokerThread->joinable())
    {
    this->BrokerThread->join();
    }
  }

  //----------------------------------------------------------------------------
  void setIsBrokering(bool t)
  {
    {
    boost::lock_guard<boost::mutex> lock(this->BrokeringStatus);
    this->BrokerIsRunning = t;
    }
  this->BrokerStatusChanged.notify_all();
  }

private:
  boost::scoped_ptr<boost::thread> BrokerThread;

  boost::mutex BrokeringStatus;
  boost::condition_variable BrokerStatusChanged;
  bool BrokerIsRunning;


};

}
}
}

namespace remus{
namespace server{

//------------------------------------------------------------------------------
Server::Server():
  PortInfo(),
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Publish( new remus::server::detail::EventPublisher() ),
  UUIDGenerator( new detail::UUIDManagement() ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( boost::make_shared<remus::server::WorkerFactory>() )
{
}

//------------------------------------------------------------------------------
Server::Server(const boost::shared_ptr<remus::server::WorkerFactoryBase>& factory):
  PortInfo(),
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Publish( new remus::server::detail::EventPublisher() ),
  UUIDGenerator( new detail::UUIDManagement() ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( factory )
{
}

//------------------------------------------------------------------------------
Server::Server(const remus::server::ServerPorts& ports):
  PortInfo( ports ),
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Publish( new remus::server::detail::EventPublisher() ),
  UUIDGenerator( new detail::UUIDManagement() ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( boost::make_shared<remus::server::WorkerFactory>() )
{
}

//------------------------------------------------------------------------------
Server::Server(const remus::server::ServerPorts& ports,
               const boost::shared_ptr<remus::server::WorkerFactoryBase>& factory):
  PortInfo( ports ),
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Publish( new remus::server::detail::EventPublisher() ),
  UUIDGenerator( new detail::UUIDManagement() ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( factory )
{
}

//------------------------------------------------------------------------------
Server::~Server()
{
  //when we are destructing the server, we first need to stop brokering
  this->stopBrokering();
}

//------------------------------------------------------------------------------
void Server::pollingRates(const remus::server::PollingRates& rates)
{

  //setup a new polling monitor. We don't want to change the SocketMonitor!
  //The SocketMonitor has lots of socket/worker tracking and changing
  //the SocketMonitor will break all of that.
  this->SocketMonitor->pollingMonitor().changeTimeOutRates(rates.minRate(),
                                                           rates.maxRate());
}

//------------------------------------------------------------------------------
remus::server::PollingRates Server::pollingRates() const
{
  remus::common::PollingMonitor monitor = this->SocketMonitor->pollingMonitor();
  const boost::int64_t low =  monitor.minTimeOut();
  const boost::int64_t high = monitor.maxTimeOut();
  return remus::server::PollingRates(low,high);
}

//------------------------------------------------------------------------------
bool Server::Brokering(Server::SignalHandling sh)
  {
  //start up signal catching before we start polling. We do this in the
  //startBrokering method since really the server isn't doing anything before
  //this point.
  if(sh == CAPTURE)
    {
    this->StartCatchingSignals();
    }

  zmq::socket_t clientChannel(*(this->PortInfo.context()),ZMQ_ROUTER);
  zmq::socket_t workerChannel(*(this->PortInfo.context()),ZMQ_ROUTER);
  zmq::socket_t statusChannel(*(this->PortInfo.context()),ZMQ_PUB);


  //attempts to bind to the sockets to the desired ports
  this->PortInfo.bindClient(&clientChannel);
  this->PortInfo.bindWorker(&workerChannel);

  //todo this needs to go into the PortInfo as it handles all binding
  //of sockets
  zmq::socketInfo<zmq::proto::tcp> default_sub("127.0.0.1",
                                               remus::SERVER_SUB_PORT);
  zmq::bindToAddress(statusChannel, default_sub);


  //tell the StatusPublisher what socket to use
  this->Publish->socketToUse(&statusChannel);

  //give to the worker factory the endpoint information so it can properly
  //setup workers. This needs to happen after the binding of the worker socket
  this->WorkerFactory->portForWorkersToUse( this->PortInfo.worker() );

  //construct the pollitems to have client and workers so that we process
  //messages from both sockets.
  zmq::pollitem_t items[2] = {
      { clientChannel, 0, ZMQ_POLLIN, 0 },
      { workerChannel, 0, ZMQ_POLLIN, 0 } };

  //keeps track of what our polling interval is, and adjusts it to
  //handle operating systems that throttle our polling.
  remus::common::PollingMonitor monitor = this->SocketMonitor->pollingMonitor();

  //keep track of current time since we last purged dead workers
  //we want to clear every dead workers every 250ms.
  boost::posix_time::ptime currentTime =
                            boost::posix_time::microsec_clock::local_time();

  const boost::int64_t deadWorkersCheckInterval(250);
  boost::posix_time::ptime whenToCheckForDeadWorkers =
                      boost::posix_time::microsec_clock::local_time() +
                      boost::posix_time::milliseconds(deadWorkersCheckInterval);

  //We need to notify the Thread management that brokering is about to start.
  //This allows the calling thread to resume, as it has been waiting for this
  //notification, and will also allow threads that have been holding on
  //waitForBrokeringToStart to resume
  Thread->setIsBrokering(true);
  while (Thread->isBrokering())
    {
    zmq::poll(&items[0], 2, static_cast<long>(monitor.current()) );
    monitor.pollOccurred();

    //update the current time
    currentTime = boost::posix_time::microsec_clock::local_time();

    if (items[0].revents & ZMQ_POLLIN)
      {
      //we need to strip the client address from the message
      zmq::SocketIdentity clientIdentity = zmq::address_recv(clientChannel);
      this->DetermineClientResponse(clientChannel, clientIdentity, workerChannel);
      }
    if (items[1].revents & ZMQ_POLLIN)
      {
      //a worker is registering
      //we need to strip the worker address from the message
      zmq::SocketIdentity workerIdentity = zmq::address_recv(workerChannel);
      this->DetermineWorkerResponse(workerChannel,workerIdentity);
      }

    //only purge dead workers every 250ms to reduce server load
    if(whenToCheckForDeadWorkers <= currentTime)
      {

      this->CheckForExpiredWorkersAndJobs();

      whenToCheckForDeadWorkers = currentTime +
                      boost::posix_time::milliseconds(deadWorkersCheckInterval);
      }

    //see if we have a worker in the pool for the next job in the queue,
    //otherwise as the factory to generate a new worker to handle that job
    if(Thread->isBrokering())
      {
      this->FindWorkerForQueuedJob( workerChannel );
      }
    }

  this->Publish->stop();

  //this should only happen with interrupted threads is hit; lets make sure we close
  //down all workers.
  this->WorkerFactory->setMaxWorkerCount(0);
  this->TerminateAllWorkers( workerChannel );

  if(sh == CAPTURE)
    {
    this->StopCatchingSignals();
    }

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
  return this->Thread->stop();
}

//------------------------------------------------------------------------------
bool Server::isBrokering() const
{
  return this->Thread->isBrokering();
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
void Server::DetermineClientResponse(zmq::socket_t& clientChannel,
                                     const zmq::SocketIdentity& clientIdentity,
                                     zmq::socket_t& workerChannel)
{
  remus::proto::Message msg = remus::proto::receive_Message(&clientChannel);
  //server response is the general response message type
  //the client can than convert it to the expected type
  if(!msg.isValid())
    {
    //send an invalid response.
    remus::proto::send_NonBlockingResponse(remus::INVALID_SERVICE,
                                           remus::INVALID_MSG,
                                           &clientChannel,
                                           clientIdentity);
    return; //no need to continue
    }


  //the service and data to return as a response. In case
  //of not being able to handle the given service types,
  //we will send back INVALID_SERVICE as the service type
  remus::SERVICE_TYPE response_service = msg.serviceType();
  std::string response_data;

  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case remus::SUPPORTED_IO_TYPES:
      //returns what MeshIOTypes the server supports
      //by checking the worker pool and factory
      response_data = this->allSupportedMeshIOTypes(msg);
      break;
    case remus::CAN_MESH_IO_TYPE:
      //returns if we can mesh a given MeshIOType
      //by checking the worker pool and factory
      response_data = this->canMesh(msg);
      break;
    case remus::CAN_MESH_REQUIREMENTS:
      //returns if we can mesh a given proto::JobRequirements
      //by checking the worker pool and factory
      response_data = this->canMeshRequirements(msg);
      break;
    case remus::MESH_REQUIREMENTS_FOR_IO_TYPE:
      //Generates all the JobRequirments that have the
      //passed in MeshIOType. Does this
      //by checking the worker pool and factory
      response_data = this->meshRequirements(msg);
      break;
    case remus::MAKE_MESH:
      //queues the proto::JobSubmission and returns
      //a proto::Job that can be used to track that job
      response_data = this->queueJob(msg);
      break;
    case remus::MESH_STATUS:
      //retrieves the current status of the job related to the passed
      //proto::Job. Returns a proto::JobStatus
      response_data = this->meshStatus(msg);
      break;
    case remus::RETRIEVE_RESULT:
      //retrieves the current result of the job related to the passed
      //proto::Job. Returns a proto::JobResult. The result is than deleted
      //from the server.
      //If no result exists will return an invalid JobResult
      response_data = this->retrieveResult(msg);
      break;
    case remus::TERMINATE_JOB:
      //Will try to terminate the given proto::Job.
      //If the job is currently queued on the server it will be eliminated
      //if the job is been given to a worker, we will ask the worker to
      //terminate the job. As long as the job is in the workers task
      //queue the job will be removed. If the job is currently being processed
      //we can do nothing to stop it
      response_data = this->terminateJob(workerChannel,msg);
      break;
    default:
      response_service = remus::INVALID_SERVICE;
      response_data = remus::INVALID_MSG;
    }

  //now that we have the proper service_type and data send it in a non
  //blocking manner so the server doesn't stall out sending to a client
  //that has disconnected
  remus::proto::send_NonBlockingResponse(response_service, response_data,
                                         &clientChannel,   clientIdentity);
  return;
}

//------------------------------------------------------------------------------
std::string Server::allSupportedMeshIOTypes(const remus::proto::Message& )
{
  //we ask the worker factory and Worker Pool for the MeshIO types for
  //all workers they know about
  remus::common::MeshIOTypeSet supportedTypes, poolTypes;

  supportedTypes = this->WorkerFactory->supportedIOTypes();
  poolTypes = this->WorkerPool->supportedIOTypes();

  //combine the two sets to get all the valid requirements
  supportedTypes.insert(poolTypes.begin(),poolTypes.end());
  std::ostringstream buffer;
  buffer << supportedTypes;
  return buffer.str();
}


//------------------------------------------------------------------------------
std::string Server::canMesh(const remus::proto::Message& msg)
{
  //we state that the factory can support a mesh type by having a worker
  //registered to it that supports the mesh type.
  bool workerSupport =
    (this->WorkerFactory->workerRequirements(msg.MeshIOType()).size() > 0) &&
    (this->WorkerFactory->maxWorkerCount() > 0);

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
  bool workerSupport = this->WorkerFactory->haveSupport(reqs) &&
                      (this->WorkerFactory->maxWorkerCount() > 0);

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
            this->WorkerFactory->workerRequirements(msg.MeshIOType()));

  //Query the worker pool to get the set of requirements for waiting
  //workers that support the given mesh type info
  remus::proto::JobRequirementsSet poolSet =
            this->WorkerPool->waitingWorkerRequirements(msg.MeshIOType());

  //combine the two sets to get all the valid requirements
  reqSet.insert(poolSet.begin(),poolSet.end());

  std::ostringstream buffer;
  buffer << reqSet;
  return buffer.str();
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
  const boost::uuids::uuid jobUUID = (*this->UUIDGenerator)();

  //create a new job to place on the queue
  const remus::proto::JobSubmission submission =
                  remus::proto::to_JobSubmission(msg.data(),msg.dataSize());

  this->QueuedJobs->addJob(jobUUID,submission);


  const remus::proto::Job validJob(jobUUID,msg.MeshIOType());

  //publish the job has been queued
  this->Publish->jobQueued(validJob, submission.requirements() );

  //return the UUID
  return remus::proto::to_string(validJob);
}

//------------------------------------------------------------------------------
std::string Server::retrieveResult(const remus::proto::Message& msg)
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
std::string Server::terminateJob(zmq::socket_t& workerChannel,
                                 const remus::proto::Message& msg)
{
  remus::proto::Job job = remus::proto::to_Job(msg.data(),msg.dataSize());

  const bool currentlyInQueue = this->QueuedJobs->haveUUID(job.id());
  const bool currentlyActive = this->ActiveJobs->haveUUID(job.id());
  const bool eligableForTermination = currentlyInQueue || currentlyActive;

  if(!eligableForTermination)
    {
    //state that the job can't be terminated since it is not active
    //or queued ( either an invalid job id or job is completed )
    remus::proto::JobStatus jstatus(job.id(),remus::INVALID_STATUS);
    return remus::proto::to_string(jstatus);
    }

  remus::proto::JobStatus jstatus(job.id(),remus::FAILED);
  if(currentlyInQueue)
    {
    this->QueuedJobs->remove(job.id());

    //publish that this job is now terminated and what it's last status was
    remus::proto::JobStatus lastStatus(job.id(),remus::QUEUED);
    this->Publish->jobTerminated(lastStatus);
    }
  else
    {
    //send an out of band message to the worker to terminate a job
    //if the job is in the worker queue it will be removed, if the worker
    //is currently processing the job, we will just ignore the result
    //when they are submitted
    zmq::SocketIdentity worker = this->ActiveJobs->workerAddress(job.id());
    const remus::proto::JobStatus lastStatus = this->ActiveJobs->status(job.id());

    detail::send_terminateJob(job.id(), workerChannel, worker);

    //publish that this terminate call was sent to to the worker, and
    //what was the last status we had for the job
    this->Publish->jobTerminated(lastStatus, worker);
    }

  return remus::proto::to_string(jstatus);
}

//------------------------------------------------------------------------------
void Server::DetermineWorkerResponse(zmq::socket_t& workerChannel,
                                     const zmq::SocketIdentity &workerIdentity)
{
  remus::proto::Message msg = remus::proto::receive_Message(&workerChannel);
  //if we have an invalid message just ignore it
  if(!msg.isValid())
    {
    return;
    }

  //Everything but TERMINATE_WORKER must have a msg payload
  if( (msg.serviceType() != TERMINATE_WORKER) &&
      (msg.dataSize() == 0))
    {
    return;
    }

  //we have a valid job, determine what to do with it
  switch(msg.serviceType())
    {
    case remus::CAN_MESH_REQUIREMENTS:
      {
      //convert the message into a proto::JobRequirements and add the
      //worker to the pool stating it can support the Requirements.
      //to response is required to this
      const remus::proto::JobRequirements reqs =
            remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
      this->WorkerPool->addWorker(workerIdentity,reqs);
      this->Publish->workerRegistered(workerIdentity, reqs);
      }
      break;
    case remus::MAKE_MESH:
      {
      //Mark that the given worker is ready to accept a job with the passed
      //in set of requirements
      //The worker is waiting for us to respond to the service call
      const remus::proto::JobRequirements reqs =
            remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
      this->WorkerPool->readyForWork(workerIdentity,reqs);
      this->Publish->workerReady(workerIdentity, reqs);
      }
      break;
    case remus::MESH_STATUS:
      //store the mesh status msg which is a proto::JobStatus
      //no response needed
      this->storeMeshStatus(workerIdentity, msg);
      break;
    case remus::RETRIEVE_RESULT:
      //we need to store the mesh result, no response needed
      //store mesh does it's own notification
      this->storeMesh(workerIdentity, msg);
      {
      //now that we have stored the mesh we can tell the worker
      //we have the results, which allows it to shutdown.
      //We can't have a worker shutdown before the results are
      //on the server or the results might be dropped by zmq
      //linger settings
      remus::proto::send_NonBlockingResponse(remus::RETRIEVE_RESULT,
                                             remus::INVALID_MSG,
                                             &workerChannel,
                                             workerIdentity);
      }
      break;
    case remus::HEARTBEAT:
      //pass along to the worker monitor what worker just sent a heartbeat
      //message. The heartbeat message contains the msec delta for when
      //to next expect a heartbeat message from the given worker
      { //scoped so we don't get jump bypasses variable initialization errors
      const std::string msgPayload(msg.data(),msg.dataSize());
      boost::int64_t dur_in_milli = boost::lexical_cast<boost::int64_t>(
                                                              msgPayload);
      this->SocketMonitor->heartbeat(workerIdentity,dur_in_milli);
      this->Publish->workerHeartbeat(workerIdentity);
      }
      break;
    case remus::TERMINATE_WORKER:
      //we have found out the worker is dead, dead since it has told
      //us itself that it is shutting down. We don't need to do anything
      //else as the WorkerPool and ActiveJobs will find out about the dead
      //worker by asking the SocketMonitor
      this->SocketMonitor->markAsDead(workerIdentity);
      this->Publish->workerTerminated(workerIdentity);
    default:

      break;
    }
  //if we are anything but a heartbeat message we need to refresh
  //the worker. Not the cleanest logic but I don't have a better idea
  //on how to handle this
  if(msg.serviceType() != remus::HEARTBEAT &&
     msg.serviceType() != remus::TERMINATE_WORKER)
    {
    this->SocketMonitor->refresh(workerIdentity);
    }
}

//------------------------------------------------------------------------------
void Server::storeMeshStatus(const zmq::SocketIdentity &workerIdentity,
                             const remus::proto::Message& msg)
{
  //the string in the data is actually a job status object
  remus::proto::JobStatus js = remus::proto::to_JobStatus(msg.data(),
                                                          msg.dataSize());
  this->ActiveJobs->updateStatus(js);

  this->Publish->jobStatus(js, workerIdentity);
}

//------------------------------------------------------------------------------
void Server::storeMesh(const zmq::SocketIdentity &workerIdentity,
                       const remus::proto::Message& msg)
{
  remus::proto::JobResult jr = remus::proto::to_JobResult(msg.data(),
                                                            msg.dataSize());
  this->ActiveJobs->updateResult(jr);

  this->Publish->jobFinished(jr, workerIdentity);
}

//------------------------------------------------------------------------------
void Server::assignJobToWorker(zmq::socket_t& workerChannel,
                               const zmq::SocketIdentity &workerIdentity,
                               const remus::worker::Job& job )
{
  this->ActiveJobs->add( workerIdentity, job.id() );

  remus::proto::Response response =
        remus::proto::send_NonBlockingResponse(remus::MAKE_MESH,
                                               remus::worker::to_string(job),
                                               &workerChannel,
                                               workerIdentity);
  if(response.isValid())
    { //consider sending the job to be refreshing the worker
    this->SocketMonitor->refresh(workerIdentity);

    //we should encode the worker id as part of the string
    std::string wi(workerIdentity.data(), workerIdentity.size());
    this->Publish->jobSentToWorker(job, workerIdentity);
    }

}

//see if we have a worker in the pool for the next job in the queue,
//otherwise ask the factory to generate a new worker to handle that job
//------------------------------------------------------------------------------
void Server::FindWorkerForQueuedJob(zmq::socket_t& workerChannel)
{
  //We assume that a worker could possibly handle multiple jobs but all of the same type.
  //In order to prevent allocating more workers than needed we use a set instead of a vector.
  //This results in the server only creating one worker per job type.
  //This gives the new workers the opportunity of getting assigned multiple jobs.
  this->WorkerFactory->updateWorkerCount();

  if(this->QueuedJobs->numJobsWaitingForWorkers() == 0 &&
     this->QueuedJobs->numJobsJustQueued() == 0)
    {
    //we have no jobs waiting for work so no need to do
    //all the heavy queries below
    return;
    }

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
      this->assignJobToWorker(workerChannel,
                              this->WorkerPool->takeWorker(*type),
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
      this->assignJobToWorker(workerChannel,
                              this->WorkerPool->takeWorker(*type),
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
    if(this->WorkerFactory->createWorker(*type,
                           WorkerFactoryBase::KillOnFactoryDeletion))
      {
      this->QueuedJobs->workerDispatched(*type);
      }
    }
}

//------------------------------------------------------------------------------
void Server::CheckForExpiredWorkersAndJobs()
{
//mark all jobs whose worker haven't sent a heartbeat in time
  //as a job that failed. We are returned the set of job's that are
  //expired
  std::vector< remus::proto::JobStatus > expiredJobs =
              this->ActiveJobs->markExpiredJobs((*this->SocketMonitor));

  //publish the jobs that have failed
  this->Publish->jobsExpired( expiredJobs );

  //purge all pending workers that have been explicitly termianted
  //with a TERMINATE service call. No need to publish this
  //as we do that when the service call comes in. This also updates
  //the responsive state of all workers.
  // detail::ChangedWorkers updatedWorkers =
          this->WorkerPool->purgeDeadWorkers((*this->SocketMonitor));

  // for( worker : updatedWorkers.workers())
  //   {
  //   if( worker->responsive() )
  //     {
  //     this->Publish->workerResponsive(  worker->address()  );
  //     }
  //   else
  //     {
  //     this->Publish->workerUnresponsive(  worker->address() );
  //     }
  //   }

  // <socketIdentity, new_state>
  //what we really want is to send notification for every worker whose
  //state has changed since the last time we did a dead workers check
  //that way we announce on the RESPONSIVE channel who has zombied
  //and who has been cured

  //this means that what we want is to grab all workers status before the purge

  //than after the purge regather the status and check the difference.
  //changes would be:
  //  1. Dead
  //  2. Zombie
  //  3. Alive
}

//We are crashing we need to terminate all workers
//------------------------------------------------------------------------------
void Server::signalCaught( SignalCatcher::SignalType )
{
  this->Thread->stop();

  //Remove everything from the job queue so no new jobs start up.
  this->QueuedJobs->clear();
}

//------------------------------------------------------------------------------
//terminate all workers that are doing jobs or waiting for jobs

void Server::TerminateAllWorkers( zmq::socket_t& workerChannel )
{

  //next we take workers from the worker pool and kill them all off
  std::set<zmq::SocketIdentity> pendingWorkers =
                                              this->WorkerPool->allResponsiveWorkers();

  typedef std::set<zmq::SocketIdentity>::const_iterator iterator;
  for(iterator i=pendingWorkers.begin(); i != pendingWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = (*this->UUIDGenerator)();

    detail::send_terminateWorker(jobId, workerChannel, *i);
    }

  //lastly we will kill any still active worker
  std::set<zmq::SocketIdentity> activeWorkers =
                                        this->ActiveJobs->activeWorkers();

  //only call terminate again on workers that are active
  for(iterator i=activeWorkers.begin(); i != activeWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = (*this->UUIDGenerator)();
    detail::send_terminateWorker(jobId, workerChannel, *i);
    }

}


}
}
