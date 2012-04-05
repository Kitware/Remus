/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_worker_h
#define __meshserver_worker_h

#include <zmq.hpp>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>

//forward declare boost::thread
namespace boost { class thread; }

namespace meshserver{
namespace worker{
class Worker
{
public:
  //constuct a worker that can mesh a single type
  Worker(meshserver::MESH_TYPE mtype);
  virtual ~Worker();

  //gets back a job from the broker
  //this will lock the worker as it will wait on a job message
  virtual meshserver::common::JobDetails getJob();

  //update the status of the worker
  virtual void updateStatus(const meshserver::common::JobStatus& info);

  //send to the server the mesh results.
  virtual void returnMeshResults(const meshserver::common::JobResult& result);

protected:
  //start communication. Currently is called by
  //the constructor
  bool startCommunicationThread();

  //currently called by the destructor
  bool stopCommunicationThread();
private:

  //holds the type of mesh we support
  const meshserver::MESH_TYPE MeshType;

  zmq::context_t Context;

  //this socket is used to talk to broker
  zmq::socket_t BrokerComm;

  //this is the thread that handles talking with the broker
  //and dealing with heartbeats
  class BrokerCommunicator;
  BrokerCommunicator *BComm;
  boost::thread* BrokerCommThread;
};

}
}
#endif
