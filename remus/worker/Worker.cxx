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


#include <remus/worker/Worker.h>

#include <boost/thread.hpp>
#include <string>

#include <remus/common/Message.h>
#include <remus/common/Response.h>
#include <remus/common/zmqHelper.h>

namespace remus{
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
    zmq::socket_t Worker(*context,ZMQ_PAIR);
    zmq::socket_t Server(*context,ZMQ_DEALER);

    zmq::connectToAddress(Server,this->ServerEndpoint);
    zmq::connectToAddress(Worker,this->MainEndpoint);

    zmq::pollitem_t items[2]  = { { Worker,  0, ZMQ_POLLIN, 0 },
                                  { Server,  0, ZMQ_POLLIN, 0 } };
    while( this->ContinueTalking)
      {
      zmq::poll(&items[0],2,remus::HEARTBEAT_INTERVAL);
      bool sentToServer=false;
      if(items[0].revents & ZMQ_POLLIN)
        {
        sentToServer = true;
        remus::common::Message message(Worker);

        //just pass the message on to the server
        message.send(Server);

        //special case is that shutdown means we stop looping
        if(message.serviceType()==remus::SHUTDOWN)
          {
          this->ContinueTalking = false;
          }
        }
      if(items[1].revents & ZMQ_POLLIN && this->ContinueTalking)
        {
        //we make sure ContinueTalking is valid so we know the worker
        //is still listening and not in destructor

        //we can have two  response types, one which is for the Service MakeMesh
        //and the other for the Service shutdown which kills the worker process
        remus::common::Response response(Server);
        if(response.serviceType() != remus::SHUTDOWN)
          {
          response.send(Worker);
          }
        else
          {
          //Todo: we should allow the worker to add in a hook to only terminate
          //a single job or handle doing a clean shutdown
          exit(1);
          }
        }
      if(!sentToServer)
        {
        //std::cout << "send heartbeat to server" << std::endl;
        //send a heartbeat to the server
        remus::common::Message message(remus::MESH_TYPE(),remus::HEARTBEAT);
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
Worker::Worker(remus::MESH_TYPE mtype,
               remus::worker::ServerConnection const& conn):
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
  //std::cout << "Ending the worker" << std::endl;
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
    remus::common::Message canMesh(this->MeshType,remus::CAN_MESH);
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
    remus::common::Message shutdown(this->MeshType,remus::SHUTDOWN);
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
remus::Job Worker::getJob()
{
  remus::common::Message askForMesh(this->MeshType,remus::MAKE_MESH);
  askForMesh.send(this->ServerComm);

  //we have our message back
  remus::common::Response response(this->ServerComm);
  //std::cout << "have our job from the server com in the real worker" <<std::endl;

  //we need a better serialization technique
  std::string msg = response.dataAs<std::string>();
  //std::cout << "Raw job details " << msg << std::endl;

  return remus::to_Job(msg);
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const remus::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = remus::to_string(info);
  remus::common::Message message(this->MeshType,
                                    remus::MESH_STATUS,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const remus::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = remus::to_string(result);
  remus::common::Message message(this->MeshType,
                                    remus::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}


}
}
