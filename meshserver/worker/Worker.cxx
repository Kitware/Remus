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

#include <meshserver/common/JobMessage.h>
#include <meshserver/common/JobResponse.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver{
namespace worker{

//-----------------------------------------------------------------------------
class Worker::ServerCommunicator
{
public:
  ServerCommunicator(const std::string& serverInfo,
                     const std::string& mainThreadInfo):
    ContinueTalking(true),
    ServerEndpoint(serverInfo),
    MainEndpoint(mainThreadInfo)
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

    zmq::connectToAddress(Worker,this->ServerEndpoint);
    zmq::connectToAddress(Server,this->MainEndpoint);


    zmq::pollitem_t items[2]  = { { Worker,  0, ZMQ_POLLIN, 0 },
                                  { Server,  0, ZMQ_POLLIN, 0 } };
    while( this->ContinueTalking)
      {
      zmq::poll(&items[0],2,meshserver::HEARTBEAT_INTERVAL);
      bool sentToServer=false;
      if(items[0].revents & ZMQ_POLLIN)
        {
        sentToServer = true;
        meshserver::common::JobMessage message(Worker);

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
        meshserver::common::JobResponse response(Server);

        std::cout << "sending the response" << std::endl;
        response.send(Worker);
        }
      if(!sentToServer)
        {
        std::cout << "send heartbeat to server" << std::endl;
        //send a heartbeat to the server
        meshserver::common::JobMessage message(meshserver::INVALID_MESH,meshserver::HEARTBEAT);
        message.send(Server);
        }
      }
  }

private:
  bool ContinueTalking;
  std::string ServerEndpoint;
  std::string MainEndpoint;
};

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype,
               meshserver::worker::ServerConnection const& conn):
  MeshType(mtype),
  Context(1),
  ServerComm(Context,ZMQ_PAIR),
  BComm(NULL),
  ServerCommThread(NULL)
{
  //FIRST THREAD HAS TO BIND THE INPROC SOCKET
  zmq::socketInfo<zmq::proto::inproc> internalCommInfo =
      zmq::bindToAddress<zmq::proto::inproc>(this->ServerComm,"worker");

  this->startCommunicationThread(conn.endpoint(),internalCommInfo.endpoint());
}


//-----------------------------------------------------------------------------
Worker::~Worker()
{
  std::cout << "Ending the worker" << std::endl;
  this->stopCommunicationThread();
}

//-----------------------------------------------------------------------------
bool Worker::startCommunicationThread(const std::string &serverEndpoint,
                                      const std::string &commEndpoint)
{
  if(!this->ServerCommThread && !this->BComm)
    {
    //start about the server communication thread
    this->BComm = new Worker::ServerCommunicator(serverEndpoint,commEndpoint);
    this->ServerCommThread = new boost::thread(&Worker::ServerCommunicator::run,
                                             this->BComm,&this->Context);

    //register with the server that we can mesh a certain type
    meshserver::common::JobMessage canMesh(this->MeshType,meshserver::CAN_MESH);
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
    meshserver::common::JobMessage shutdown(this->MeshType,meshserver::SHUTDOWN);
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
meshserver::JobDetails Worker::getJob()
{
  meshserver::common::JobMessage askForMesh(this->MeshType,meshserver::MAKE_MESH);
  askForMesh.send(this->ServerComm);

  //we have our message back
  meshserver::common::JobResponse response(this->ServerComm);
  std::cout << "have our job from the server com in the real worker" <<std::endl;

  //we need a better serialization techinque
  std::string msg = response.dataAs<std::string>();
  std::cout << "Raw job details " << msg << std::endl;

  return meshserver::to_JobDetails(msg);
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const meshserver::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = meshserver::to_string(info);
  meshserver::common::JobMessage message(this->MeshType,
                                    meshserver::MESH_STATUS,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const meshserver::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = meshserver::to_string(result);
  meshserver::common::JobMessage message(this->MeshType,
                                    meshserver::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}


}
}
