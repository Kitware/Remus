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


#include <meshserver/worker/Worker.h>

#include <boost/thread.hpp>
#include <string>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver{
namespace worker{

//-----------------------------------------------------------------------------
class Worker::ServerCommunicator
{
public:
  ServerCommunicator():
    ContinueTalking(true)
    {
    }

  ~ServerCommunicator()
  {
  }

  void run(zmq::context_t *context)
  {
    std::cout << "thread started" << std::endl;

    zmq::socket_t Worker(*context,ZMQ_PAIR);
    zmq::socket_t Server(*context,ZMQ_DEALER);

    Worker.connect("inproc://worker");
    zmq::connectToSocket(Server,meshserver::BROKER_WORKER_PORT);


    zmq::pollitem_t items[2]  = { { Worker,  0, ZMQ_POLLIN, 0 },
                                  { Server,  0, ZMQ_POLLIN, 0 } };
    while( this->ContinueTalking)
      {
      zmq::poll(&items[0],2,meshserver::HEARTBEAT_INTERVAL);
      bool sentToServer=false;
      if(items[0].revents & ZMQ_POLLIN)
        {
        sentToServer = true;
        meshserver::JobMessage message(Worker);

        //just pass the message on to the server
        message.send(Server);

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

        //we have a message from the server for know this can only be the
        //reply back from a job query so send it to the worker.
        meshserver::JobResponse response(Server);

        std::cout << "sending the response" << std::endl;
        response.send(Worker);
        }
      if(!sentToServer)
        {
        std::cout << "send heartbeat to server" << std::endl;
        //send a heartbeat to the server
        meshserver::JobMessage message(meshserver::INVALID_MESH,meshserver::HEARTBEAT);
        message.send(Server);
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
  ServerComm(Context,ZMQ_PAIR),
  BComm(NULL),
  ServerCommThread(NULL)
{
  //FIRST THREAD HAS TO BIND THE INPROC SOCKET
  this->ServerComm.bind("inproc://worker");

  this->startCommunicationThread();
}

//-----------------------------------------------------------------------------
Worker::~Worker()
{
  std::cout << "Ending the worker" << std::endl;
  this->stopCommunicationThread();
}

//-----------------------------------------------------------------------------
bool Worker::startCommunicationThread()
{
  if(!this->ServerCommThread && !this->BComm)
    {
    //start about the server communication thread
    this->BComm = new Worker::ServerCommunicator();
    this->ServerCommThread = new boost::thread(&Worker::ServerCommunicator::run,
                                             this->BComm,&this->Context);

    //register with the server that we can mesh a certain type
    meshserver::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);
    canMesh.send(this->ServerComm);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool Worker::stopCommunicationThread()
{
  if(this->ServerCommThread && this->BComm)
    {
    //send message that we are shuting down communication
    meshserver::JobMessage shutdown(this->MeshType,meshserver::SHUTDOWN);
    shutdown.send(this->ServerComm);

    this->ServerCommThread->join();
    delete this->BComm;
    delete this->ServerCommThread;

    this->BComm = NULL;
    this->ServerCommThread = NULL;
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
meshserver::common::JobDetails Worker::getJob()
{
  meshserver::JobMessage askForMesh(this->MeshType,meshserver::MAKE_MESH);
  askForMesh.send(this->ServerComm);

  //we have our message back
  meshserver::JobResponse response(this->ServerComm);
  std::cout << "have our job from the server com in the real worker" <<std::endl;

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
  message.send(this->ServerComm);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const meshserver::common::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = meshserver::to_string(result);
  meshserver::JobMessage message(this->MeshType,
                                    meshserver::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}


}
}
