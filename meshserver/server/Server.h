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
#include <meshserver/common/zmqHelper.h>

#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>

#include <meshserver/server/WorkerFactory.h>
#include <meshserver/common/meshServerGlobals.h>


//included for symbol exports
#include "ServerExports.h"

namespace meshserver{
//forward declaration of classes only the implementation needs
  namespace common{
  class JobDetails;
  }
class JobMessage;
class JobQueue;
}

namespace meshserver{
namespace server{
  namespace internal
    {
    //forward declaration of classes only the implementation needs
    class ActiveJobs;
    class JobQueue;
    class WorkerPool;
    }

class MESHSERVERSERVER_EXPORT Server
{
public:
  Server();
  explicit Server(const meshserver::server::WorkerFactory& factory);

  ~Server();
  bool startBrokering();

  const zmq::socketInfo<zmq::proto::tcp>& clientSocketInfo() const
    {return ClientSocketInfo;}
  const zmq::socketInfo<zmq::proto::tcp>& workerSocketInfo() const
    {return WorkerSocketInfo;}

protected:
  //processes all job queries
  void DetermineJobQueryResponse(const zmq::socketIdentity &clientIdentity,
                                 const meshserver::JobMessage& msg);

  //These methods are all to do with send responses to job messages
  bool canMesh(const meshserver::JobMessage& msg);
  std::string meshStatus(const meshserver::JobMessage& msg);
  std::string queueJob(const meshserver::JobMessage& msg);
  std::string retrieveMesh(const meshserver::JobMessage& msg);

  //Methods for processing Worker queries
  void DetermineWorkerResponse(const zmq::socketIdentity &workerIdentity,
                              const meshserver::JobMessage& msg);
  void storeMeshStatus(const meshserver::JobMessage& msg);
  void storeMesh(const meshserver::JobMessage& msg);
  void assignJobToWorker(const zmq::socketIdentity &workerIdentity,
                         const meshserver::common::JobDetails& job);

  void FindWorkerForQueuedJob();

private:
  zmq::context_t Context;
  zmq::socket_t ClientQueries;
  zmq::socket_t WorkerQueries;

//allow subclasses to override these internal containers
protected:
  boost::uuids::random_generator UUIDGenerator;
  boost::scoped_ptr<meshserver::server::internal::JobQueue> QueuedJobs;
  boost::scoped_ptr<meshserver::server::internal::WorkerPool> WorkerPool;
  boost::scoped_ptr<meshserver::server::internal::ActiveJobs> ActiveJobs;

  meshserver::server::WorkerFactory WorkerFactory;
  zmq::socketInfo<zmq::proto::tcp> ClientSocketInfo;
  zmq::socketInfo<zmq::proto::tcp> WorkerSocketInfo;
};

}
}


#endif
