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

#ifndef __remus_server_Server_h
#define __remus_server_Server_h

#include <remus/common/SignalCatcher.h>
#include <remus/common/zmqHelper.h>

#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>

#include <remus/server/WorkerFactory.h>
#include <remus/server/ServerPorts.h>

//included for symbol exports
#include <remus/server/ServerExports.h>

namespace remus {
  //forward declaration of classes only the implementation needs
  namespace common {
  class Message;
  }

  namespace worker {
  class Job;
  }
}

namespace remus{
namespace server{
  namespace detail
    {
    //forward declaration of classes only the implementation needs
    class ActiveJobs;
    class JobQueue;
    class WorkerPool;
    }

//Server is the broker of Remus. It handles accepting client
//connections, worker connections, and manages the life cycle of submitted jobs.
//We inherit from SignalCatcher so that we can properly handle
//segfaults and other abnormal termination conditions
//The Server class doesn't support copy or move semantics
class REMUSSERVER_EXPORT Server : public remus::common::SignalCatcher
{
public:
  //construct a new server using the default worker factory
  //and default loopback ports
  Server();

  //construct a new server with a custom factory
  //and the default loopback ports
  explicit Server(const remus::server::WorkerFactory& factory);

  //construct a new server using the given loop back ports
  //and the default factory
  explicit Server(remus::server::ServerPorts ports);

  //construct a new server using the given loop back ports
  //and the default factory
  Server(remus::server::ServerPorts ports,
                  const remus::server::WorkerFactory& factory);

  //cleanup the server
  virtual ~Server();

  //when you call start brokering the server will actually start accepting
  //worker and client requests.
  virtual bool startBrokering();

  //get back the port information that this server bound too. Since multiple
  //remus servers can be running at a single time this is a way for the server
  //to report which port it bound it self too.
  const remus::server::ServerPorts& ServerPortInfo() const {return PortInfo;}

  //control how we handle abnormal signals that we catch
  virtual void SignalCaught( SignalCatcher::SignalType signal );

protected:
  //processes all job queries
  void DetermineJobQueryResponse(const zmq::socketIdentity &clientIdentity,
                                 const remus::common::Message& msg);

  //These methods are all to do with send responses to job messages
  bool canMesh(const remus::common::Message& msg);
  std::string meshStatus(const remus::common::Message& msg);
  std::string queueJob(const remus::common::Message& msg);
  std::string retrieveMesh(const remus::common::Message& msg);
  std::string terminateJob(const remus::common::Message& msg);

  //Methods for processing Worker queries
  void DetermineWorkerResponse(const zmq::socketIdentity &workerIdentity,
                              const remus::common::Message& msg);
  void storeMeshStatus(const remus::common::Message& msg);
  void storeMesh(const remus::common::Message& msg);
  void assignJobToWorker(const zmq::socketIdentity &workerIdentity,
                         const remus::worker::Job& job);

  //see if we have a worker in the pool for the next job in the queue,
  //otherwise ask the factory to generate a new worker to handle that job
  //virtual so that people using custom factories can decide the lifespan
  //of workers
  //overriding this will also allow custom servers to change the priority
  //of queued jobs and workers
  virtual void FindWorkerForQueuedJob();

  //terminate all workers that are doing jobs or waiting for jobs
  void TerminateAllWorkers();

protected:
  //allow subclasses to override these detail containers
  zmq::context_t Context;
  zmq::socket_t ClientQueries;
  zmq::socket_t WorkerQueries;

  boost::uuids::random_generator UUIDGenerator;
  boost::scoped_ptr<remus::server::detail::JobQueue> QueuedJobs;
  boost::scoped_ptr<remus::server::detail::WorkerPool> WorkerPool;
  boost::scoped_ptr<remus::server::detail::ActiveJobs> ActiveJobs;

  remus::server::WorkerFactory WorkerFactory;
  remus::server::ServerPorts PortInfo;
};

}

typedef remus::server::Server Server;
}


#endif
