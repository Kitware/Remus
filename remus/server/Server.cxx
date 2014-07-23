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
#include <remus/server/detail/JobQueue.h>
#include <remus/server/detail/SocketMonitor.h>
#include <remus/server/detail/WorkerPool.h>

#include <set>


//initialize the static instance variable in signal catcher in the class
//that inherits from it
remus::common::SignalCatcher* remus::common::SignalCatcher::Instance = NULL;

namespace remus{
namespace server{
namespace detail{

//------------------------------------------------------------------------------
remus::proto::Response make_terminateWorker(boost::uuids::uuid jobId)
{
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());

  return remus::proto::Response(remus::TERMINATE_WORKER,
                                remus::worker::to_string(terminateJob));
}

//------------------------------------------------------------------------------
remus::proto::Response make_terminateJob(boost::uuids::uuid jobId)
{
  remus::worker::Job terminateJob(jobId,
                                  remus::proto::JobSubmission());
  return remus::proto::Response(remus::TERMINATE_JOB,
                                remus::worker::to_string(terminateJob));
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
    boost::lock_guard<boost::mutex> lock(this->BrokeringStatus);
    this->BrokerIsRunning = t;
    }
  this->BrokerStatusChanged.notify_all();
  }
};

//------------------------------------------------------------------------------
struct ZmqManagement
{
  zmq::socket_t ClientQueries;
  zmq::socket_t WorkerQueries;

  //----------------------------------------------------------------------------
  ZmqManagement( const remus::server::ServerPorts& ports ):
    ClientQueries(*(ports.context()),ZMQ_ROUTER),
    WorkerQueries(*(ports.context()),ZMQ_ROUTER)
  {}
};


}
}
}

namespace remus{
namespace server{

//------------------------------------------------------------------------------
Server::Server():
  PortInfo(),
  Zmq( new detail::ZmqManagement( PortInfo )),
  UUIDGenerator(), //use default random number generator
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( boost::make_shared<remus::server::WorkerFactory>() )
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory->addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(const boost::shared_ptr<remus::server::WorkerFactory>& factory):
  PortInfo(),
  Zmq( new detail::ZmqManagement( PortInfo ) ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( factory )
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory->addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(const remus::server::ServerPorts& ports):
  PortInfo( ports ),
  Zmq( new detail::ZmqManagement(ports) ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( boost::make_shared<remus::server::WorkerFactory>() )
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory->addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::Server(const remus::server::ServerPorts& ports,
               const boost::shared_ptr<remus::server::WorkerFactory>& factory):
  PortInfo( ports ),
  Zmq( new detail::ZmqManagement(ports) ),
  UUIDGenerator(), //use default random number generator
  QueuedJobs( new remus::server::detail::JobQueue() ),
  SocketMonitor( new remus::server::detail::SocketMonitor() ),
  WorkerPool( new remus::server::detail::WorkerPool() ),
  ActiveJobs( new remus::server::detail::ActiveJobs () ),
  Thread( new detail::ThreadManagement() ),
  WorkerFactory( factory )
  {
  //attempts to bind to a tcp socket, with a prefered port number
  this->PortInfo.bindClient(&this->Zmq->ClientQueries);
  this->PortInfo.bindWorker(&this->Zmq->WorkerQueries);
  //give to the worker factory the endpoint information needed to connect to myself
  this->WorkerFactory->addCommandLineArgument(this->PortInfo.worker().endpoint());
  }

//------------------------------------------------------------------------------
Server::~Server()
{

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

  //construct the pollitems to have client and workers so that we process
  //messages from both sockets.
  zmq::pollitem_t items[2] = {
      { this->Zmq->ClientQueries,  0, ZMQ_POLLIN, 0 },
      { this->Zmq->WorkerQueries, 0, ZMQ_POLLIN, 0 } };

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

  while (Thread->isBrokering())
    {
    zmq::poll(&items[0], 2, static_cast<long>(monitor.current()) );
    monitor.pollOccurred();

    //update the current time
    currentTime = boost::posix_time::microsec_clock::local_time();

    if (items[0].revents & ZMQ_POLLIN)
      {
      //we need to strip the client address from the message
      zmq::SocketIdentity clientIdentity = zmq::address_recv(this->Zmq->ClientQueries);

      //Note the contents of the message isn't valid
      //after the DetermineJobQueryResponse call
      remus::proto::Message message(&this->Zmq->ClientQueries);
      this->DetermineJobQueryResponse(clientIdentity,message); //NOTE: this will queue jobs
      // std::cout << "c" << std::endl;
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
      // std::cout << "w" << std::endl;
      }

    //only purge dead workers every 250ms to reduce server load
    if(whenToCheckForDeadWorkers <= currentTime)
      {
      // std::cout << "checking for dead workers" << std::endl;
      //mark all jobs whose worker haven't sent a heartbeat in time
      //as a job that failed.
      this->ActiveJobs->markExpiredJobs((*this->SocketMonitor));

      //purge all pending workers with jobs that haven't sent a heartbeat
      this->WorkerPool->purgeDeadWorkers((*this->SocketMonitor));

      whenToCheckForDeadWorkers = currentTime +
                      boost::posix_time::milliseconds(deadWorkersCheckInterval);
      }

    //see if we have a worker in the pool for the next job in the queue,
    //otherwise as the factory to generat a new worker to handle that job
    this->FindWorkerForQueuedJob();
    }

  //this should only happen with interrupted threads be hit; lets make sure we close
  //down all workers.
  this->TerminateAllWorkers();

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
void Server::DetermineJobQueryResponse(const zmq::SocketIdentity& clientIdentity,
                                  const remus::proto::Message& msg)
{
  //msg.dump(std::cout);
  //server response is the general response message type
  //the client can than convert it to the expected type
  if(!msg.isValid())
    {
    //send an invalid response.
    remus::proto::Response response(remus::INVALID_SERVICE,
                                    remus::INVALID_MSG);
    response.sendNonBlocking(&this->Zmq->ClientQueries, clientIdentity);
    std::cerr << "send back INVALID_SERVICE to client " << std::endl;
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
    case remus::MAKE_MESH:
      // std::cout << "c MAKE_MESH" << std::endl;
      response_data =this->queueJob(msg);
      break;
    case remus::MESH_STATUS:
      // std::cout << "c MESH_STATUS" << std::endl;
      response_data =this->meshStatus(msg);
      break;
    case remus::CAN_MESH:
      // std::cout << "c CAN_MESH" << std::endl;
      response_data =this->canMesh(msg);
      break;
    case remus::CAN_MESH_REQUIREMENTS:
      // std::cout << "c CAN_MESH_REQS" << std::endl;
      response_data =this->canMeshRequirements(msg);
      break;
    case remus::MESH_REQUIREMENTS:
      response_data =this->meshRequirements(msg);
      break;
    case remus::RETRIEVE_MESH:
      response_data =this->retrieveMesh(msg);
      break;
    case remus::TERMINATE_JOB:
      response_data =this->terminateJob(msg);
      break;
    default:
      response_service = remus::INVALID_SERVICE;
      response_data = remus::INVALID_MSG;
    }

  //now that we have the proper service_type and data send it in a non
  //blocking manner so the server doesn't stall out sending to a client
  //that has disconnected
  remus::proto::Response response(response_service,response_data);
  response.sendNonBlocking(&this->Zmq->ClientQueries, clientIdentity);

  return;
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

    //send an out of band message to the worker to terminate a job
    //if the job is in the worker queue it will be removed, if the worker
    //is currently processing the job, we will just ignore the result
    //when they are submitted
    if(removed && worker.size() > 0)
      {
      remus::proto::Response response = detail::make_terminateJob(job.id());
      response.sendNonBlocking(&this->Zmq->WorkerQueries, worker);
      }
    }

  remus::STATUS_TYPE status = (removed) ? remus::FAILED : remus::INVALID_STATUS;
  return remus::proto::to_string(remus::proto::JobStatus(job.id(),status));
}

//------------------------------------------------------------------------------
void Server::DetermineWorkerResponse(const zmq::SocketIdentity &workerIdentity,
                                     const remus::proto::Message& msg)
{
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
    case remus::MAKE_MESH:
      {
      // std::cout << "w MAKE_MESH" << std::endl;
      const remus::proto::JobRequirements reqs =
            remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
      this->WorkerPool->readyForWork(workerIdentity,reqs);
      }
      break;
    case remus::MESH_STATUS:
      // std::cout << "w MAKE_STATUS" << std::endl;
      //store the mesh status msg,  no response needed
      this->storeMeshStatus(msg);
      break;
    case remus::CAN_MESH:
      {
      // std::cout << "w CAN_MESH" << std::endl;
      const remus::proto::JobRequirements reqs =
            remus::proto::to_JobRequirements(msg.data(),msg.dataSize());
      this->WorkerPool->addWorker(workerIdentity,reqs);
      }
      break;
    case remus::RETRIEVE_MESH:
      //we need to store the mesh result, no response needed
      this->storeMesh(msg);
      break;
    case remus::HEARTBEAT:
      // std::cout << "w HEARTBEAT" << std::endl;
      //pass along to the worker monitor what worker just sent a heartbeat
      //message
      this->SocketMonitor->heartbeat(workerIdentity,msg);
      break;
    case remus::TERMINATE_WORKER:
      //we have found out the worker is dead, dead since it has told
      //us itself that it is shutting down. We don't need to do anything
      //else as the WorkerPool and ActiveJobs will find out about the dead
      //worker by asking the SocketMonitor
      this->SocketMonitor->markAsDead(workerIdentity);
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

  remus::proto::Response response(remus::MAKE_MESH,
                                  remus::worker::to_string(job));

  response.sendNonBlocking(&this->Zmq->WorkerQueries, workerIdentity);
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
  this->WorkerFactory->updateWorkerCount();


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
    if(this->WorkerFactory->createWorker(*type,
                           WorkerFactory::KillOnFactoryDeletion))
      {
      this->QueuedJobs->workerDispatched(*type);
      }
    }
}


//We are crashing we need to terminate all workers
//------------------------------------------------------------------------------
void Server::signalCaught( SignalCatcher::SignalType )
{
  this->Thread->stop();

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
                                              this->WorkerPool->allWorkers();

  typedef std::set<zmq::SocketIdentity>::const_iterator iterator;
  for(iterator i=pendingWorkers.begin(); i != pendingWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = this->UUIDGenerator();

    remus::proto::Response response =
                          detail::make_terminateWorker(jobId);

    response.sendNonBlocking(&this->Zmq->WorkerQueries, (*i));
    }

  //lastly we will kill any still active worker
  std::set<zmq::SocketIdentity> activeWorkers =
                                        this->ActiveJobs->activeWorkers();

  //only call terminate again on workers that are active
  for(iterator i=activeWorkers.begin(); i != activeWorkers.end(); ++i)
    {
    //make a fake id and send that with the terminate command
    const boost::uuids::uuid jobId = this->UUIDGenerator();

    remus::proto::Response response =
                          detail::make_terminateWorker(jobId);

    response.sendNonBlocking(&this->Zmq->WorkerQueries, (*i));
    }

}


}
}
