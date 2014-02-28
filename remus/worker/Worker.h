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

#include <remus/proto/JobRequirements.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/zmqSocketIdentity.h>

#include <remus/worker/Job.h>
#include <remus/worker/ServerConnection.h>

//included for export symbols
#include <remus/worker/WorkerExports.h>

#include <boost/scoped_ptr.hpp>

namespace remus{
namespace worker{
  namespace detail
  {
  //forward declaration of classes only the implementation needs
  class MessageRouter;
  class JobQueue;
  struct ZmqManagement;
  }

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
  explicit Worker(remus::common::MeshIOType mtype,
                  const remus::worker::ServerConnection& conn);

  //construct a worker that can mesh only an exact set of requirements.
  //it uses the server connection object to determine what server
  //to connect too
  explicit Worker(const remus::proto::JobRequirements& requirements,
                  const remus::worker::ServerConnection& conn);

  virtual ~Worker();

  //send a message to the server stating how many jobs
  //that we want to be sent to process
  virtual void askForJobs( unsigned int numberOfJobs = 1 );

  //query to see how many pending jobs we need to process
  virtual std::size_t pendingJobCount( ) const;

  //fetch a pending job
  virtual remus::worker::Job takePendingJob();

  //Blocking fetch a pending job and return it
  virtual remus::worker::Job getJob();

  //update the status of the worker
  virtual void updateStatus(const remus::proto::JobStatus& info);

  //send to the server the mesh results.
  virtual void returnMeshResults(const remus::proto::JobResult& result);

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
