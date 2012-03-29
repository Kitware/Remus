/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_worker_h
#define __meshserver_worker_h

#include <zmq.hpp>
#include <boost/thread.hpp>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>
#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver
{
class Worker
{
public:
  //constuct a worker that can mesh a single type
  Worker(meshserver::MESH_TYPE mtype);

  //gets back a job from the broker 
  //this will lock the worker as it will wait on a job message
  meshserver::JobMessage getJob();

  //update the status of the worker
  //status is FINISHED,FAILED, CRASHED
  //this value will be cached locally and returned to the broker
  //when it asks for our current status
  void updateStatus(meshserver::STATUS_TYPE type);

  //update the progress of the worker
  //progress will be modified so that it fall between 1 and 100
  //this value will be cached locally and returned to the broker
  //when it asks for our current progress
  void updateProgress(char progress);

  //update the local cache of the mesh result.
  //this will be sent to the server when it requests our mesh result
  void setMeshResult(const std::string& result);

private:
  zmq::context_t Context;

  //this socket is used to talk to our communication thread
  zmq::socket_T CommSocket;
  boost::thread CommThread;
};


//-----------------------------------------------------------------------------
void WorkerReactor(zmq::context_t& context)
{ 
  zmq::socket_t workerComm(context,ZMQ_PAIR); //get information from worker
  zmq::socket_t brokerComm(context, ZMQ_ROUTER); //respond to the broker

  zmq::pollitem_t items[2] = {
      { workerComm,  0, ZMQ_POLLIN, 0 },
      { brokerComm, 0, ZMQ_POLLIN, 0 } };

  //items that we cache
  char progress;
  meshserver::STATUS_TYPE status;
  std::string returnAddress;
  while(true)
    {
    zmq::poll (&items[0], 2, -1);
    if (items [0].revents & ZMQ_POLLIN)
      {
      //Note: the presumption is that once we recieve a mesh from the broker
      //we send back immediately that we have a job, so that the broker
      //knows not to send us a job intill we re-register with the broker      
      returnAddress = zmq::s_recv(workerComm);
      meshserver::JobMessage message(workerComm);


      if(message.serviceType() == meshserver::MAKE_MESH)
        {
        //we have a make mesh, respond to broker that we have job

        //if we get a message to do a job from the server, we force that
        //down the workerComm channel!
        workerComm.send(message);
        }
      }
    else if (items [1].revents & ZMQ_POLLIN)
      {
      //we are getting a message from the worker, so lets cache the results
      //in this thread in case the broker asks for it.
      returnAddress = zmq::s_recv(workerComm);

      //parse the message, look for header info
      //if status store in status var,
      //if asking for job, return 

      }
    }      

};

//-----------------------------------------------------------------------------
Worker::Worker(meshserver::MESH_TYPE mtype);
  Context(1),
  CommSocket(this->Context,ZMQ_PAIR)
  {
  this->CommSocket.connect ("inproc://workerComm");

  //start up the thread and let it run forever
  this->CommThread = boost::thread(WorkerReactor,this->Context);
  }
}

#endif
