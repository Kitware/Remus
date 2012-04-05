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


    zmq::pollitem_t items[2]  = { { Worker,  0, ZMQ_POLLIN, 0 },
                                  { Broker,  0, ZMQ_POLLIN, 0 } };
    while( this->ContinueTalking)
      {
      zmq::poll(&items[0],2,meshserver::HEARTBEAT_INTERVAL);
      bool sentToBroker=false;
      if(items[0].revents & ZMQ_POLLIN)
        {
        sentToBroker = true;
        meshserver::JobMessage message(Worker);

        //just pass the message on to the broker
        message.send(Broker);

        //special case is that shutdown means we stop looping
        if(message.serviceType()==meshserver::SHUTDOWN)
          {
          this->ContinueTalking = false;
          }
        }
      if(items[1].revents & ZMQ_POLLIN && this->ContinueTalking)
        {
        //we make sure ContinueTalking is valid so we know the worker
        //is still listening and not in destructor

        std::cout << "Building a response to send to the worker" << std::endl;

        //we have a message from the broker for know this can only be the
        //reply back from a job query so send it to the worker.
        meshserver::JobResponse response(Broker);

        std::cout << "sending the response" << std::endl;
        response.send(Worker);
        }
      if(!sentToBroker)
        {
        std::cout << "send heartbeat to broker" << std::endl;
        //send a heartbeat to the broker
        meshserver::JobMessage message(meshserver::INVALID_MESH,meshserver::HEARTBEAT);
        message.send(Broker);
        }
      }
  }

private:
  bool ContinueTalking;
};

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype):
  MeshType(mtype),
  Context(1),
  BrokerComm(Context,ZMQ_PAIR),
  BComm(NULL),
  BrokerCommThread(NULL)
{
  //FIRST THREAD HAS TO BIND THE INPROC SOCKET
  this->BrokerComm.bind("inproc://worker");

  this->startCommunicationThread();
}

//-----------------------------------------------------------------------------
Worker::~Worker()
{
  this->stopCommunicationThread();
}

//-----------------------------------------------------------------------------
bool Worker::startCommunicationThread()
{
  if(!this->BrokerCommThread && !this->BComm)
    {
    //start about the broker communication thread
    this->BComm = new Worker::BrokerCommunicator();
    this->BrokerCommThread = new boost::thread(&Worker::BrokerCommunicator::run,
                                             this->BComm,&this->Context);

    //register with the broker that we can mesh a certain type
    meshserver::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);
    canMesh.send(this->BrokerComm);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool Worker::stopCommunicationThread()
{
  if(this->BrokerCommThread && this->BComm)
    {
    //send message that we are shuting down communication
    meshserver::JobMessage shutdown(this->MeshType,meshserver::SHUTDOWN);
    shutdown.send(this->BrokerComm);

    this->BrokerCommThread->join();
    delete this->BComm;
    delete this->BrokerCommThread;

    this->BComm = NULL;
    this->BrokerCommThread = NULL;
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
meshserver::common::JobDetails Worker::getJob()
{
  meshserver::JobMessage askForMesh(this->MeshType,meshserver::MAKE_MESH);
  askForMesh.send(this->BrokerComm);

  //we have our message back
  meshserver::JobResponse response(this->BrokerComm);
  std::cout << "have our job from the broker com in the real worker" <<std::endl;

  //we need a better serialization techinque
  std::string msg = response.dataAs<std::string>();
  std::cout << "Raw job details " << msg << std::endl;

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
