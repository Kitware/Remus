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

#ifndef __remus_worker_h
#define __remus_worker_h

#include <remus/common/zmqHelper.h>

#include <remus/Job.h>
#include <remus/JobResult.h>
#include <remus/JobStatus.h>

#include <remus/common/MeshIOType.h>

#include <remus/worker/ServerConnection.h>

//included for symbol exports
#include <remus/worker/WorkerExports.h>

//forward declare boost::thread
namespace boost { class thread; }

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

  //gets back a job from the server
  //this will lock the worker as it will wait on a job message
  virtual remus::Job getJob();

  //update the status of the worker
  virtual void updateStatus(const remus::JobStatus& info);

  //send to the server the mesh results.
  virtual void returnMeshResults(const remus::JobResult& result);

protected:
  //start communication. Currently is called by
  //the constructor
  bool startCommunicationThread(const std::string &serverEndpoint,
                                const std::string &commEndpoint);

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
  boost::thread* ServerCommThread;
};

}

//We want the user to have a nicer experience creating the worker interface.
//For this reason we remove the stuttering when making an instance of the worker.
typedef remus::worker::Worker Worker;

}
#endif
