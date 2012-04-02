/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_worker_h
#define __meshserver_worker_h

#include <zmq.hpp>
#include <string>
#include <set>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <csignal>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver
{
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
};


int Worker::CaughtCrashSignal = 0;
boost::condition_variable Worker::CrashCond;
boost::mutex Worker::CrashMutex;

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype):
  MeshType(mtype),
  Context(1),
  Broker(this->Context,ZMQ_DEALER),
  CurrentJobIds()
{
  zmq::connectToSocket(this->Broker,meshserver::BROKER_WORKER_PORT);

  //setup exception handling CTRL+C etc failures
  this->setupCrashHandling();

  //setup the thread that will report back to the broker
  this->ExceptionHandlingThread = new boost::thread(&Worker::reportCrash,this);

}

//-----------------------------------------------------------------------------
Worker::~Worker()
{
  delete this->ExceptionHandlingThread;
}

//-----------------------------------------------------------------------------
void Worker::setupCrashHandling()
{
  struct sigaction action;
  action.sa_handler = Worker::crashHandler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT,&action,NULL);
  sigaction(SIGTERM,&action,NULL);
}

//-----------------------------------------------------------------------------
void Worker::crashHandler(int value)
{
  boost::lock_guard<boost::mutex> lock(Worker::CrashMutex);
  Worker::CaughtCrashSignal = value;
  Worker::CrashCond.notify_all();
}

//-----------------------------------------------------------------------------
void Worker::reportCrash()
{

  boost::unique_lock<boost::mutex> lock(Worker::CrashMutex);
  while(this->CaughtCrashSignal==0)
    {
    Worker::CrashCond.wait(lock);
    }

  for(CJ_It i=this->CurrentJobIds.begin();
      i != this->CurrentJobIds.end(); ++i)
    {
    const meshserver::common::JobStatus js(*i, meshserver::FAILED);
    const std::string msg(meshserver::to_string(js));
    meshserver::JobMessage jm(this->MeshType,meshserver::MESH_STATUS,msg.data(),msg.size());
    jm.send(this->Broker);
    }
  exit(this->CaughtCrashSignal);
}

//-----------------------------------------------------------------------------
meshserver::common::JobDetails Worker::getJob()
{
  //send to the client that we are ready for a job.
  meshserver::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);

  zmq::pollitem_t item =  { this->Broker,  0, ZMQ_POLLIN, 0 };

  while(true)
    {
    //send the request for a mesh
    canMesh.send(this->Broker);
    zmq::poll(&item,1,-1);
    if(item.revents & ZMQ_POLLIN)
      {
      //we have our message back
      meshserver::JobResponse response(this->Broker);

      //we need a better serialization techinque
      std::string msg = response.dataAs<std::string>();

      //see if we are given a job, or a request to poll again
      if(msg != meshserver::INVALID_MSG)
        {
        const meshserver::common::JobDetails temp = meshserver::to_JobDetails(msg);
        this->CurrentJobIds.insert(temp.JobId);
        return temp;
        }      
      }
    }
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const meshserver::common::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = meshserver::to_string(info);
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::MESH_STATUS,
                                    msg.data(),msg.size());
  message.send(this->Broker);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const meshserver::common::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = meshserver::to_string(result);
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->Broker);

  //remove the id from our current list of jobs we are doing
  this->CurrentJobIds.erase(result.JobId);
}


}
#endif
