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
                     const std::string& mainThreadInfo,
                     const std::string& jobQueueInfo):
    ServerCommThread(NULL),
    ContinueTalking(true),
    ServerEndpoint(serverInfo),
    MainEndpoint(mainThreadInfo),
    JobQueueEndpoint(jobQueueInfo)
    {
    }

  ~ServerCommunicator()
  {
  if(this->ServerCommThread)
    {
    delete this->ServerCommThread;
    this->ServerCommThread = NULL;
    }
  }

  bool isTalking() const
  {
  boost::unique_lock<boost::mutex>(TalkingMutex);
  return ContinueTalking;
  }

  void stopTalking()
  {
  boost::unique_lock<boost::mutex>(TalkingMutex);
  ContinueTalking = false;
  }

  void run(zmq::context_t *context)
  {
    zmq::socket_t Worker(*context,ZMQ_PAIR);
    zmq::socket_t JobQueue(*context,ZMQ_PAIR);

    zmq::socket_t Server(*context,ZMQ_DEALER);

    zmq::connectToAddress(Server,this->ServerEndpoint);
    zmq::connectToAddress(Worker,this->MainEndpoint);
    zmq::connectToAddress(JobQueue,this->JobQueueEndpoint);

    zmq::pollitem_t items[2]  = { { Worker,  0, ZMQ_POLLIN, 0 },
                                  { Server,  0, ZMQ_POLLIN, 0 } };
    while( this->isTalking() )
      {
      zmq::poll(&items[0],2,remus::HEARTBEAT_INTERVAL);
      bool sentToServer=false;
      if(items[0].revents & ZMQ_POLLIN)
        {
        sentToServer = true;
        remus::common::Message message(Worker);

        //just pass the message on to the server
        message.send(Server);

        //special case is that TERMINATE_JOB_AND_WORKER means we stop looping
        if(message.serviceType()==remus::TERMINATE_WORKER)
          {
          this->stopTalking();
          }
        }
      if(items[1].revents & ZMQ_POLLIN && this->isTalking())
        {
        //we make sure ContinueTalking is valid so we know the worker
        //is still listening and not in destructor

        //we can have two  response types, one which is for the Service MakeMesh
        //and the other for the Service TERMINATE_JOB_AND_WORKER which kills
        //the worker process
        remus::common::Response response(Server);
        switch(response.serviceType())
          {
          case remus::TERMINATE_WORKER:
            this->stopTalking();
          case remus::TERMINATE_JOB:
          case remus::MAKE_MESH:
            //send the job to the queue so that somebody can take it later
            response.send(JobQueue);
            break;
          default:
            response.send(Worker);
          }
        }
      if(!sentToServer)
        {
        //std::cout << "send heartbeat to server" << std::endl;
        //send a heartbeat to the server
        remus::common::Message message(remus::common::MeshIOType(),remus::HEARTBEAT);
        message.send(Server);
        }
      }
  }

  boost::thread* ServerCommThread;

private:
  bool ContinueTalking;
  boost::mutex TalkingMutex;
  std::string ServerEndpoint;
  std::string MainEndpoint;
  std::string JobQueueEndpoint;
};

//-----------------------------------------------------------------------------
Worker::Worker(remus::common::MeshIOType mtype,
               remus::worker::ServerConnection const& conn):
  MeshIOType(mtype),
  Context(1),
  ServerComm(Context,ZMQ_PAIR),
  BComm(NULL),
  JobQueue( Context ),
  ConnectedToLocalServer( conn.isLocalEndpoint() )
{
  //FIRST THREAD HAS TO BIND THE INPROC SOCKET
  zmq::socketInfo<zmq::proto::inproc> internalCommInfo =
      zmq::bindToAddress<zmq::proto::inproc>(this->ServerComm,"worker");

  this->startCommunicationThread(conn.endpoint(),
                                 internalCommInfo.endpoint(),
                                 this->JobQueue.endpoint());
}


//-----------------------------------------------------------------------------
Worker::~Worker()
{
  //std::cout << "Ending the worker" << std::endl;
  this->stopCommunicationThread();
}

//-----------------------------------------------------------------------------
bool Worker::startCommunicationThread(const std::string &serverEndpoint,
                                      const std::string &commEndpoint,
                                      const std::string &jobQueueEndpoint)
{
  if(!this->BComm)
    {
    //start about the server communication thread
    this->BComm = new Worker::ServerCommunicator(serverEndpoint,
                                                 commEndpoint,
                                                 jobQueueEndpoint);
    this->BComm->ServerCommThread =
             new boost::thread(&Worker::ServerCommunicator::run,
                               this->BComm,
                               &this->Context);

    //register with the server that we can mesh a certain type
    remus::common::Message canMesh(this->MeshIOType,remus::CAN_MESH);
    canMesh.send(this->ServerComm);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool Worker::stopCommunicationThread()
{
  if(this->BComm)
    {
    //send message that we are shutting down communication, and we can stop
    //polling the server
    if(this->BComm->isTalking())
    {
      remus::common::Message shutdown(this->MeshIOType,
                                      remus::TERMINATE_WORKER);
      shutdown.send(this->ServerComm);
    }

    this->BComm->ServerCommThread->join();
    delete this->BComm;
    this->BComm = NULL;
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void Worker::askForJobs( unsigned int numberOfJobs )
{
  remus::common::Message askForMesh(this->MeshIOType,remus::MAKE_MESH);
  for(int i=0; i < numberOfJobs; ++i)
    { askForMesh.send(this->ServerComm); }
}

//-----------------------------------------------------------------------------
std::size_t Worker::pendingJobCount() const
{
  return this->JobQueue.size();
}

//-----------------------------------------------------------------------------
remus::worker::Job Worker::takePendingJob()
{
  return this->JobQueue.take();
}

//-----------------------------------------------------------------------------
remus::worker::Job Worker::getJob()
{
  if(this->pendingJobCount() == 0)
    {
    this->askForJobs(1);
    while(this->pendingJobCount() == 0) {}
    }
  return this->JobQueue.take();
}

//-----------------------------------------------------------------------------
void Worker::updateStatus(const remus::worker::JobStatus& info)
{
  //send a message that contains, the status
  std::string msg = remus::worker::to_string(info);
  remus::common::Message message(this->MeshIOType,
                                    remus::MESH_STATUS,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}

//-----------------------------------------------------------------------------
void Worker::returnMeshResults(const remus::worker::JobResult& result)
{
  //send a message that contains, the path to the resulting file
  std::string msg = remus::worker::to_string(result);
  remus::common::Message message(this->MeshIOType,
                                    remus::RETRIEVE_MESH,
                                    msg.data(),msg.size());
  message.send(this->ServerComm);
}

//-----------------------------------------------------------------------------
void Worker::terminateWorker()
{
  exit(1);
}


}
}
