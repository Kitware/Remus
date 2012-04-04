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
    ContinueTalking(true)
    {
    }

  ~BrokerCommunicator()
  {
  }

  void run(zmq::context_t *context)
  {
    std::cout << "thread started" << std::endl;

    zmq::socket_t Worker(*context,ZMQ_PAIR);
    zmq::socket_t Broker(*context,ZMQ_DEALER);

    Worker.connect("inproc://worker");
    zmq::connectToSocket(Broker,meshserver::BROKER_WORKER_PORT);

    zmq::pollitem_t item  = { Worker,  0, ZMQ_POLLIN, 0 };

    while(this->ContinueTalking)
      {
      zmq::poll(&item,1,meshserver::HEARTBEAT_INTERVAL);
      if(item.revents & ZMQ_POLLIN)
        {
        std::cout << "got msg from worker" << std::endl;
        meshserver::JobMessage message(Worker);
        //just pass the message on to the broker
        message.send(Broker);
        if(message.serviceType() == meshserver::MAKE_MESH)
          {

          //if this is a get a mesh request, we need to send the
          //data to the worker
          meshserver::JobResponse response(Broker);
          std::cout << "sending job back to worker from broker" << std::endl;
          response.send(Worker);
          }
        }
      else
        {
        std::cout << "send heartbeat to broker" << std::endl;
        //send a heartbeat to the broker
        meshserver::JobMessage message(meshserver::INVALID_MESH,meshserver::HEARTBEAT);
        message.send(Broker);
        }
      }

    std::cout << "somehow we stopped polling" << std::endl;
    //close now
  }

  void stop()
  {
    this->ContinueTalking = false;
  }

private:
  bool ContinueTalking;
};

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype):
  MeshType(mtype),
  Context(1),
  BrokerComm(Context,ZMQ_PAIR)
{
  //FIRST THREAD HAS TO BIND THE INPROC SOCKET
  this->BrokerComm.bind("inproc://worker");
  //start about the broker communication thread
  this->BComm = new Worker::BrokerCommunicator();
  this->BrokerCommThread = new boost::thread(&Worker::BrokerCommunicator::run,
                                             this->BComm,&this->Context);


  //register with the broker that we can mesh a certain type
  meshserver::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);
  canMesh.send(this->BrokerComm);
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
  meshserver::JobMessage askForMesh(this->MeshType,meshserver::MAKE_MESH);
  askForMesh.send(this->BrokerComm);

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
