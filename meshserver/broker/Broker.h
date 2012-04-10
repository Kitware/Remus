/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __broker_h
#define __broker_h

#include <zmq.hpp>
#include <meshserver/common/zmqHelper.h>

#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>

#include <meshserver/broker/WorkerFactory.h>
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
namespace broker{
  namespace internal
    {
    //forward declaration of classes only the implementation needs
    class ActiveJobs;
    class JobQueue;
    class WorkerPool;
    }

class Broker
{
public:
  Broker();
  Broker(const meshserver::broker::WorkerFactory& factory);

  ~Broker();
  bool startBrokering();

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
  void assignJobToWorker(const zmq::socketAddress &workAddress,
                         const meshserver::common::JobDetails& job);

  void FindWorkerForQueuedJob();

private:
  boost::uuids::random_generator UUIDGenerator;

  boost::scoped_ptr<meshserver::broker::internal::JobQueue> QueuedJobs;
  boost::scoped_ptr<meshserver::broker::internal::WorkerPool> WorkerPool;
  boost::scoped_ptr<meshserver::broker::internal::ActiveJobs> ActiveJobs;
  meshserver::broker::WorkerFactory WorkerFactory;

  zmq::context_t Context;
  zmq::socket_t JobQueries;
  zmq::socket_t WorkerQueries;
};

}
}


#endif
