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

#include <remus/common/zmqHelper.h>

#include <remus/worker/Job.h>
#include <remus/worker/JobResult.h>
#include <remus/worker/JobStatus.h>

#include <remus/common/MeshIOType.h>

#include <remus/worker/ServerConnection.h>
#include <remus/worker/detail/JobQueue.h>

//included for symbol exports
#include <remus/worker/WorkerExports.h>

namespace remus{
namespace worker{

//The worker class is the interface for accepting jobs to process from a
// remus server. Once you get a job from the server you process the given
// job reporting back the status of the job, and than once finished the
// results of the job.
class REMUSWORKER_EXPORT Worker
{
public:
  //construct a worker that can mesh a single type
  //it uses the server connection object to determine what server
  //to connect too
  explicit Worker(remus::common::MeshIOType mtype,
                  remus::worker::ServerConnection const& conn);

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
  virtual void updateStatus(const remus::worker::JobStatus& info);

  //send to the server the mesh results.
  virtual void returnMeshResults(const remus::worker::JobResult& result);

protected:
  //start communication. Currently is called by
  //the constructor
  bool startCommunicationThread(const std::string &serverEndpoint,
                                const std::string &commEndpoint,
                                const std::string &jobQueueEndpoint);

  //currently called by the destructor
  bool stopCommunicationThread();
private:

  //holds the type of mesh we support
  const remus::common::MeshIOType MeshIOType;

  zmq::context_t Context;

  //this socket is used to talk to server
  zmq::socket_t ServerComm;

  //this is the thread that handles talking with the server
  //and dealing with heartbeats
  class ServerCommunicator;
  ServerCommunicator *BComm;

  remus::worker::detail::JobQueue JobQueue;

  bool ConnectedToLocalServer;
};

}

//We want the user to have a nicer experience creating the worker interface.
//For this reason we remove the stuttering when making an instance of the worker.
typedef remus::worker::Worker Worker;

}
#endif
