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

#ifndef remus_server_Server_h
#define remus_server_Server_h

#include <remus/common/CompilerInformation.h>
#include <remus/common/SignalCatcher.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <remus/server/WorkerFactoryBase.h>
#include <remus/server/ServerPorts.h>

//included for export symbols
#include <remus/server/ServerExports.h>


//forward declaration of classes only the implementation needs
namespace zmq { struct SocketIdentity; }

namespace remus {
  //forward declaration of classes only the implementation needs
  namespace proto {
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
    class SocketMonitor;
    class WorkerPool;
    class EventPublisher;

    struct ThreadManagement;
    struct UUIDManagement;
    }

//helper class that allows users to set and get the polling rates for
//a server instance
class REMUSSERVER_EXPORT PollingRates
{
public:
  PollingRates(boost::int64_t min_millisec, boost::int64_t max_mill):
    MinRateMillisec(min_millisec),
    MaxRateMillisec(max_mill)
    {
    }

  const boost::int64_t& minRate() const { return MinRateMillisec; }
  const boost::int64_t& maxRate() const { return MaxRateMillisec; }

private:
  boost::int64_t MinRateMillisec;
  boost::int64_t MaxRateMillisec;
};


//Server is the broker of Remus. It handles accepting client
//connections, worker connections, and manages the life cycle of submitted jobs.
//We inherit from SignalCatcher so that we can properly handle
//segfaults and other abnormal termination conditions
//The Server class doesn't support copy or move semantics
class REMUSSERVER_EXPORT Server : public remus::common::SignalCatcher
{
public:
  friend struct remus::server::detail::ThreadManagement;
  enum SignalHandling {NONE, CAPTURE};
  //construct a new server with the default worker factory and server ports.
  Server();

  //construct a new server with a custom factory and default server ports.
  explicit Server(const boost::shared_ptr<remus::server::WorkerFactoryBase>& factory);

  //construct a new server using the given server ports and default factory.
  explicit Server(const remus::server::ServerPorts& ports);

  //construct a new server using the given server ports and factory.
  Server(const remus::server::ServerPorts& ports,
         const boost::shared_ptr<remus::server::WorkerFactoryBase>& factory);

  //cleanup the server
  virtual ~Server();

  //Modify the polling interval rates for the server. The server uses a
  //a dynamic polling monitor that adjusts the frequency of the polling rate
  //based on the amount of traffic it receives. These controls allow users
  //to determine what the floor and ceiling are on the polling timeout rates.
  //If you are creating a server that requires a short lifespan or
  //needs to be highly responsive, changing these will be required.
  //
  //Note: All rates are in milliseconds
  //Note: will return false if rates are non positive values
  void pollingRates( const remus::server::PollingRates& rates );
  remus::server::PollingRates pollingRates() const;

  //when you call start brokering the server will actually start accepting
  //worker and client requests.
  //IMPORTANT:
  //A server only binds to the required server ports once brokering has
  //been started.
  bool startBrokering(SignalHandling sh = CAPTURE);

  //when you call start brokering the server will actually start accepting
  //worker and client requests. This is an easy helper to start with
  //signal handling enabled
  //IMPORTANT:
  //A server only binds to the required server ports once brokering has
  //been started.
  bool startBrokeringWithSignalHandling()
    { return startBrokering(CAPTURE); }

  //when you call start brokering the server will actually start accepting
  //worker and client requests. This is an easy helper to start without
  //signal handling enabled
  //IMPORTANT:
  //A server only binds to the required server ports once brokering has
  //been started.
  bool startBrokeringWithoutSignalHandling()
    { return startBrokering(NONE); }

  //when you call stop brokering, the server will stop accepting worker
  //and client requests. This will also tell all active workers that we
  //are shutting down, so they themselves will terminate. You can't stop
  //the server and expect 'good' things to happen.
  //IMPORTANT:
  //Once you call stopBrokering the server unbinds from its given server ports.
  //That means if you stop and restart a server it might rebind to new ports
  //if another program / server has bound to the ports while you are stopped.
  void stopBrokering();

  //Returns if the server is still brokering client and worker requests
  bool isBrokering() const;

  //Waits until the thread is up and running
  void waitForBrokeringToStart();

  //Waits until brokering starts up and finishes. Make sure to check isBrokering
  //before calling this if you waiting on a server to shutdown
  void waitForBrokeringToFinish();

  //get back the port information that this server bound too. Since multiple
  //remus servers can be running at a single time this is a way for the server
  //to report which port it bound it self too.
  //IMPORTANT:
  //A Remus server only reports which ports it has properly bound to after
  //you start it brokering. If you query serverPortInfo before a server has
  //started brokering you will get what ports the server desires to bind to.
  const remus::server::ServerPorts& serverPortInfo() const {return PortInfo;}

  //control how we handle abnormal signals that we catch
  virtual void signalCaught( SignalCatcher::SignalType signal );

protected:
  //The main brokering loop, called by thread
  virtual bool Brokering(SignalHandling sh = CAPTURE);

  //processes all client queries
  void DetermineClientResponse(zmq::socket_t& clientChannel,
                               const zmq::SocketIdentity &clientIdentity,
                               zmq::socket_t& WorkerChannel);

  //These methods are all to do with sending responses to clients
  std::string allSupportedMeshIOTypes(const remus::proto::Message& msg);
  std::string canMesh(const remus::proto::Message& msg);
  std::string canMeshRequirements(const remus::proto::Message& msg);
  std::string meshRequirements(const remus::proto::Message& msg);
  std::string meshStatus(const remus::proto::Message& msg);
  std::string queueJob(const remus::proto::Message& msg);
  std::string retrieveResult(const remus::proto::Message& msg);
  std::string terminateJob(zmq::socket_t& WorkerChannel,const remus::proto::Message& msg);

  //Methods for processing Worker queries
  void DetermineWorkerResponse(zmq::socket_t& clientChannel,
                               const zmq::SocketIdentity &workerIdentity,
                               bool& workerTerminated);

  //These methods are all to do with sending/recving to workers
  void storeMeshStatus(const zmq::SocketIdentity &workerIdentity,
                       const remus::proto::Message& msg);
  void storeMesh(const zmq::SocketIdentity &workerIdentity,
                 const remus::proto::Message& msg);
  void assignJobToWorker(zmq::socket_t& workerChannel,
                         const zmq::SocketIdentity &workerIdentity,
                         const remus::worker::Job& job);

  //see if we have a worker in the pool for the next job in the queue,
  //otherwise ask the factory to generate a new worker to handle that job
  //virtual so that people using custom factories can decide the lifespan
  //of workers
  //overriding this will also allow custom servers to change the priority
  //of queued jobs and workers
  virtual void FindWorkerForQueuedJob(zmq::socket_t& workerChannel);

  //remove any job that has expired, remove workers that have
  //stated they are shutting down, mark workers that are
  //not sending messages back to the server, update the worker factory
  //for changes, and lastly publish this all through our event publisher
  void CheckForChangeInWorkersAndJobs();

  //terminate all workers that are doing jobs or waiting for jobs
  void TerminateAllWorkers(zmq::socket_t& workerChannel);

private:
  //explicitly state the server doesn't support copy or move semantics
  Server(const Server&);
  void operator=(const Server&);

  remus::server::ServerPorts PortInfo;

  boost::scoped_ptr<remus::server::detail::JobQueue> QueuedJobs;
  boost::scoped_ptr<remus::server::detail::SocketMonitor> SocketMonitor;
  boost::scoped_ptr<remus::server::detail::WorkerPool> WorkerPool;
  boost::scoped_ptr<remus::server::detail::ActiveJobs> ActiveJobs;

  boost::scoped_ptr<remus::server::detail::EventPublisher> Publish;

  boost::scoped_ptr<detail::UUIDManagement> UUIDGenerator;
  boost::scoped_ptr<detail::ThreadManagement> Thread;

protected:
  //needs to be a shared_ptr since we can be passed in a WorkerFactoryBase
  boost::shared_ptr<remus::server::WorkerFactoryBase> WorkerFactory;
};

}

typedef remus::server::Server Server;
}


#endif
