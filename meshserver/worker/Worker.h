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

#ifndef __meshserver_worker_h
#define __meshserver_worker_h

#include <meshserver/common/zmqHelper.h>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>

#include <meshserver/worker/ServerConnection.h>

//included for symbol exports
#include "WorkerExports.h"

//forward declare boost::thread
namespace boost { class thread; }

namespace meshserver{
namespace worker{
class MESHSERVERWORKER_EXPORT Worker
{
public:
  //constuct a worker that can mesh a single type
  //it uses the server connection object to determine what server
  //to connect too
  explicit Worker(meshserver::MESH_TYPE mtype,
                  meshserver::worker::ServerConnection const& conn);

  virtual ~Worker();

  //gets back a job from the server
  //this will lock the worker as it will wait on a job message
  virtual meshserver::common::JobDetails getJob();

  //update the status of the worker
  virtual void updateStatus(const meshserver::common::JobStatus& info);

  //send to the server the mesh results.
  virtual void returnMeshResults(const meshserver::common::JobResult& result);

protected:
  //start communication. Currently is called by
  //the constructor
  bool startCommunicationThread(const std::string &serverEndpoint,
                                const std::string &commEndpoint);

  //currently called by the destructor
  bool stopCommunicationThread();
private:

  //holds the type of mesh we support
  const meshserver::MESH_TYPE MeshType;

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
}
#endif
