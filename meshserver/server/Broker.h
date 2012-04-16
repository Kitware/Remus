/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __server_h
#define __server_h

#include <zmq.hpp>
#include <meshserver/common/zmqHelper.h>

#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>

#include <meshserver/server/WorkerFactory.h>
#include <meshserver/common/meshServerGlobals.h>

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

class Server
{
public:
  Server();
  explicit Server(const meshserver::server::WorkerFactory& factory);

  ~Server();
  bool startServering();

protected:
  //processes all job queries
  void DetermineJobQueryResponse(const zmq::socketAddress &clientAddress,
                                 const meshserver::JobMessage& msg);

  //These methods are all to do with send responses to job messages
  bool canMesh(const meshserver::JobMessage& msg);
  std::string meshStatus(const meshserver::JobMessage& msg);
  std::string queueJob(const meshserver::JobMessage& msg);
  std::string retrieveMesh(const meshserver::JobMessage& msg);

  //Methods for processing Worker queries
  void DetermineWorkerResponse(const zmq::socketAddress &workAddress,
                              const meshserver::JobMessage& msg);
  void storeMeshStatus(const meshserver::JobMessage& msg);
  void storeMesh(const meshserver::JobMessage& msg);
  void assignJobToWorker(const zmq::socketAddress &workerAddress,
                         const meshserver::common::JobDetails& job);

  void FindWorkerForQueuedJob();

private:
  boost::uuids::random_generator UUIDGenerator;

  boost::scoped_ptr<meshserver::server::internal::JobQueue> QueuedJobs;
  boost::scoped_ptr<meshserver::server::internal::WorkerPool> WorkerPool;
  boost::scoped_ptr<meshserver::server::internal::ActiveJobs> ActiveJobs;
  meshserver::server::WorkerFactory WorkerFactory;

  zmq::context_t Context;
  zmq::socket_t JobQueries;
  zmq::socket_t WorkerQueries;
};

}
}


#endif
