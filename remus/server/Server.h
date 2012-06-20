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

#ifndef __server_h
#define __server_h

#include <zmq.hpp>
#include <remus/common/zmqHelper.h>

#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>

#include <remus/server/WorkerFactory.h>
#include <remus/common/remusGlobals.h>


//included for symbol exports
#include "ServerExports.h"

namespace remus{
//forward declaration of classes only the implementation needs
  namespace common{
  class JobMessage;
  }
  class Job;
}

namespace remus{
namespace server{
  namespace internal
    {
    //forward declaration of classes only the implementation needs
    class ActiveJobs;
    class JobQueue;
    class WorkerPool;
    }

//Server is the broker of Remus. It handles accepting client
//connections, worker connections, and manages the life cycle of submitted jobs.
class REMUSSERVER_EXPORT Server
{
public:
  //construct a new server using the default worker factory
  Server();

  //construct a new server with a custom factory
  explicit Server(const remus::server::WorkerFactory& factory);

  ~Server();

  //when you call start brokering the server will actually start accepting
  //worker and client requests.
  bool startBrokering();

  //get back the port information that this server bound too. Since multiple
  //remus servers can be running at a single time this is a way for the server
  //to report which port it bound it self too. This call gets the exact port
  //the the server is listening to client requests on
  const zmq::socketInfo<zmq::proto::tcp>& clientSocketInfo() const
    {return ClientSocketInfo;}

  //get back the port information that this server bound too. Since multiple
  //remus servers can be running at a single time this is a way for the server
  //to report which port it bound it self too. This call gets the exact port
  //the the server is listening to worker requests on
  const zmq::socketInfo<zmq::proto::tcp>& workerSocketInfo() const
    {return WorkerSocketInfo;}

protected:
  //processes all job queries
  void DetermineJobQueryResponse(const zmq::socketIdentity &clientIdentity,
                                 const remus::common::JobMessage& msg);

  //These methods are all to do with send responses to job messages
  bool canMesh(const remus::common::JobMessage& msg);
  std::string meshStatus(const remus::common::JobMessage& msg);
  std::string queueJob(const remus::common::JobMessage& msg);
  std::string retrieveMesh(const remus::common::JobMessage& msg);
  std::string terminateJob(const remus::common::JobMessage& msg);

  //Methods for processing Worker queries
  void DetermineWorkerResponse(const zmq::socketIdentity &workerIdentity,
                              const remus::common::JobMessage& msg);
  void storeMeshStatus(const remus::common::JobMessage& msg);
  void storeMesh(const remus::common::JobMessage& msg);
  void assignJobToWorker(const zmq::socketIdentity &workerIdentity,
                         const remus::Job& job);

  void FindWorkerForQueuedJob();

private:
  zmq::context_t Context;
  zmq::socket_t ClientQueries;
  zmq::socket_t WorkerQueries;

//allow subclasses to override these internal containers
protected:
  boost::uuids::random_generator UUIDGenerator;
  boost::scoped_ptr<remus::server::internal::JobQueue> QueuedJobs;
  boost::scoped_ptr<remus::server::internal::WorkerPool> WorkerPool;
  boost::scoped_ptr<remus::server::internal::ActiveJobs> ActiveJobs;

  remus::server::WorkerFactory WorkerFactory;
  zmq::socketInfo<zmq::proto::tcp> ClientSocketInfo;
  zmq::socketInfo<zmq::proto::tcp> WorkerSocketInfo;
};

}

typedef remus::server::Server Server;
}


#endif
