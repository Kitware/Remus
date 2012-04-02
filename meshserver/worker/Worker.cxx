/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include <meshserver/worker/Worker.h>

#include <string>
#include <csignal>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>

namespace meshserver{
namespace worker{

int Worker::CaughtCrashSignal = 0;
boost::condition_variable Worker::CrashCond;
boost::mutex Worker::CrashMutex;
boost::mutex Worker::ExitMutex;

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
  this->ExceptionHandlingThread = new boost::thread(boost::bind(&Worker::reportCrash,this));

}

//-----------------------------------------------------------------------------
Worker::~Worker()
{
  std::cout << "called destructor " << std::endl;
  this->ExceptionHandlingThread->interrupt();
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
  {
  boost::lock_guard<boost::mutex> lock(Worker::CrashMutex);
  Worker::CaughtCrashSignal = 1;
  }
  Worker::CrashCond.notify_all();

  boost::unique_lock<boost::mutex> ulock(Worker::ExitMutex);
  while(Worker::CaughtCrashSignal==1)
    {
    Worker::CrashCond.wait(ulock);
    }
  ulock.release();
  abort();
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

  this->CaughtCrashSignal = 0;
  lock.release();
  Worker::CrashCond.notify_one();
  return;
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
}
