/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include <meshserver/worker/Worker.h>

#include <boost/thread.hpp>
#include <string>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver{
namespace worker{

//-----------------------------------------------------------------------------
class Worker::BrokerCommunicator
{
public:
  BrokerCommunicator():
    ContinueTalking(true),
    Worker(NULL),
    Broker(NULL)
    {
    }

  ~BrokerCommunicator()
  {
  if(this->Worker)
    {
    delete this->Worker;
    delete this->Broker;
    }
  }  

  void run(zmq::context_t *context)
  {
    this->Worker = new zmq::socket_t(*context,ZMQ_ROUTER);
    this->Broker = new zmq::socket_t(*context,ZMQ_DEALER);

    this->Worker->connect("inproc://worker");
    zmq::connectToSocket(*this->Broker,meshserver::BROKER_WORKER_PORT);

    zmq::pollitem_t item  = { this->Worker,  0, ZMQ_POLLIN, 0 };

    while(this->ContinueTalking)
    {
    zmq::poll(&item,1,-1);
    if(item.revents & ZMQ_POLLIN)
      {
      //a worker is registering
      //we need to strip the worker address from the message
      const std::string workerAddress = zmq::s_recv(*this->Worker);

      meshserver::JobMessage message(*this->Worker);
      if(message.serviceType() == meshserver::CAN_MESH)
        {
        bool haveValidJob = false;
        while(this->ContinueTalking && haveValidJob)
          {
          //send the message to the broker
          message.send(*this->Broker);

          //we have our message back
          meshserver::JobResponse response(*this->Broker);
          
          //we need a better serialization techinque
          std::string responseMsg = response.dataAs<std::string>();

          //see if we are given a job, or a request to poll again
          if(responseMsg != meshserver::INVALID_MSG)
            {
            response.send(*this->Worker);
            haveValidJob = true;
            }
          }
        }
      else
        {
        //just pass the message on to the broker
        message.send(*this->Broker);
        }
      }
    }
  }

  void stop()
  {
    this->ContinueTalking = false;
  }

private:
  bool ContinueTalking;
  zmq::socket_t* Worker;
  zmq::socket_t* Broker;
};

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype):
  MeshType(mtype),
  Context(1),
  BrokerComm(Context,ZMQ_DEALER)
{
  this->BrokerComm.connect("inproc://worker");
  //start about the broker communication thread
  this->BComm = new Worker::BrokerCommunicator();
  this->BrokerCommThread = new boost::thread(&Worker::BrokerCommunicator::run,
                                             this->BComm,&this->Context);
}

//-----------------------------------------------------------------------------
Worker::~Worker()
{
  this->stopCommunicatorThread();
}

//-----------------------------------------------------------------------------
void Worker::stopCommunicatorThread()
{
  if(this->BrokerCommThread && this->BComm)
    {
    this->BComm->stop();
    this->BrokerCommThread->join();
    delete this->BComm;
    delete this->BrokerCommThread;
    }
}

//-----------------------------------------------------------------------------
meshserver::common::JobDetails Worker::getJob()
{
  //send to the client that we are ready for a job.
  meshserver::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);


  canMesh.send(this->BrokerComm);
  //we have our message back
  meshserver::JobResponse response(this->BrokerComm);

  //we need a better serialization techinque
  std::string msg = response.dataAs<std::string>();

  return meshserver::to_JobDetails(msg);
}  

//-----------------------------------------------------------------------------
void Worker::updateStatus(const meshserver::common::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = meshserver::to_string(info);
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::MESH_STATUS,
                                    msg.data(),msg.size());
  message.send(this->BrokerComm);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const meshserver::common::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = meshserver::to_string(result);
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->BrokerComm);
}


}
}
