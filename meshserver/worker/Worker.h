/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_worker_h
#define __meshserver_worker_h

#include <zmq.hpp>
#include <set>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver{
namespace worker{
class Worker
{
public:
  //constuct a worker that can mesh a single type
  Worker(meshserver::MESH_TYPE mtype);
  ~Worker();

  //gets back a job from the broker
  //this will lock the worker as it will wait on a job message
  meshserver::common::JobDetails getJob();

  //update the status of the worker
  void updateStatus(const meshserver::common::JobStatus& info);

  //send to the server the mesh results.
  void returnMeshResults(const meshserver::common::JobResult& result);

private:
  //setups the signal handling for exceptions
  //these allow us to register to a static variable that we should
  //handle a crash.
  void setupCrashHandling();
  static void crashHandler(int value);

  //holds the type of mesh we support
  const meshserver::MESH_TYPE MeshType;

  zmq::context_t Context;

  //this socket is used to talk to broker
  zmq::socket_t Broker;

  //holds all the job ids this worker is processing
  typedef std::set<boost::uuids::uuid>::const_iterator CJ_It;
  std::set<boost::uuids::uuid> CurrentJobIds;

  //this is the thread that handles notify the broker when we catch
  //a signal that means we have to die
  boost::thread* ExceptionHandlingThread;
  void reportCrash(); //method the thread uses to report to broker

  static int CaughtCrashSignal;
  static boost::condition_variable CrashCond;
  static boost::mutex CrashMutex;
  static boost::mutex ExitMutex;
};

}
}
#endif
