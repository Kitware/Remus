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

#ifndef remus_worker_h
#define remus_worker_h

#include <remus/common/MeshIOType.h>

//Workers include everything from proto, so that
//users don't need as many includes
#include <remus/proto/JobRequirements.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>

#include <remus/worker/Job.h>
#include <remus/worker/ServerConnection.h>

#include <boost/scoped_ptr.hpp>

//included for export symbols
#include <remus/worker/WorkerExports.h>

namespace remus{
namespace worker{
  namespace detail
  {
  //forward declaration of classes only the implementation needs
  class MessageRouter;
  class JobQueue;
  struct ZmqManagement;
  }

//helper class that allows users to set and get the polling rates for
//a server instance
class REMUSWORKER_EXPORT PollingRates
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


//The worker class is the interface for accepting jobs to process from a
// remus server. Once you get a job from the server you process the given
// job reporting back the status of the job, and than once finished the
// results of the job.
class REMUSWORKER_EXPORT Worker
{
public:
  //construct a worker that can mesh a single type
  //it uses the server connection object to determine what server
  //to connect too. The requirements of this worker are extremely simple
  //in that it only demands a given mesh input and output type.
  Worker(remus::common::MeshIOType mtype,
         const remus::worker::ServerConnection& conn);

  //construct a worker that can mesh only an exact set of requirements.
  //it uses the server connection object to determine what server
  //to connect too
  Worker(const remus::proto::JobRequirements& requirements,
         const remus::worker::ServerConnection& conn);

  virtual ~Worker();

  //return the connection info that was used to connect to the
  //remus server
  const remus::worker::ServerConnection& connection() const;

  //Modify the polling interval rates for the worker. The worker uses a
  //a dynamic polling monitor that adjusts the frequency of the polling rate
  //based on the amount of traffic it receives. These controls allow users
  //to determine what the floor and ceiling are on the polling timeout rates.
  //If you are creating a server that requires a short lifespan or
  //needs to be highly responsive, changing these will be required.
  //
  //Note: All rates are in milliseconds
  //Note: will return false if rates are non positive values
  void pollingRates( const remus::worker::PollingRates& rates );
  remus::worker::PollingRates pollingRates() const;

  //send a message to the server stating how many jobs
  //that we want to be sent to process
  void askForJobs( unsigned int numberOfJobs = 1 );

  //query to see how many pending jobs we need to process
  std::size_t pendingJobCount( ) const;

  //fetch a pending job, if no jobs are pending will return
  //an invalid job, this method will never block waiting for a job
  //from the server
  remus::worker::Job takePendingJob();

  //Blocking fetch a pending job and return it
  remus::worker::Job getJob();

  //update the status of the worker
  void updateStatus(const remus::proto::JobStatus& info);

  //send to the server the mesh results.
  void returnResult(const remus::proto::JobResult& result);

  //ask the worker API if the server has told us we should shutdown.
  //This means that the server has shutdown and all jobs the worker
  //has are invalid and can be terminated.
  bool workerShouldTerminate() const;

  //ask if a current processing job should be terminated, which can
  //happen if the client sends a terminate call while the worker is processing
  //the job
  bool jobShouldBeTerminated( const remus::worker::Job& job ) const;

private:
  //holds the type of mesh we support
  const remus::proto::JobRequirements MeshRequirements;

  remus::worker::ServerConnection ConnectionInfo;

  boost::scoped_ptr<detail::ZmqManagement> Zmq;
  boost::scoped_ptr<remus::worker::detail::MessageRouter> MessageRouter;
  boost::scoped_ptr<remus::worker::detail::JobQueue> JobQueue;

  //explicitly state the worker doesn't support copy or move semantics
  Worker(const Worker&);
  void operator=(const Worker&);
};

}

//We want the user to have a nicer experience creating the worker interface.
//For this reason we remove the stuttering when making an instance of the worker.
typedef remus::worker::Worker Worker;

}
#endif
