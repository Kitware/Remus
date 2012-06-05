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
#include <remus/common/meshServerGlobals.h>


//included for symbol exports
#include "ServerExports.h"

namespace remus{
//forward declaration of classes only the implementation needs
  namespace common{
  class JobMessage;
  }
  class JobDetails;
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

class REMUSSERVER_EXPORT Server
{
public:
  Server();
  explicit Server(const remus::server::WorkerFactory& factory);

  ~Server();
  bool startBrokering();

  const zmq::socketInfo<zmq::proto::tcp>& clientSocketInfo() const
    {return ClientSocketInfo;}
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

  //Methods for processing Worker queries
  void DetermineWorkerResponse(const zmq::socketIdentity &workerIdentity,
                              const remus::common::JobMessage& msg);
  void storeMeshStatus(const remus::common::JobMessage& msg);
  void storeMesh(const remus::common::JobMessage& msg);
  void assignJobToWorker(const zmq::socketIdentity &workerIdentity,
                         const remus::JobDetails& job);

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
}


#endif
