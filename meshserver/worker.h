/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __worker_h
#define __worker_h

#include <zmq.hpp>
#include "Common/meshServerGlobals.h"
#include "Common/zmqHelper.h"

#include <sstream>
#include <iostream>

namespace meshserver
{
class Worker
{
public:

Worker():
  Context(1),
  MeshJobs(this->Context, ZMQ_PULL),
  MeshStatus(this->Context, ZMQ_PUSH)
  {
  zmq::connectToSocket(this->MeshJobs,meshserver::BROKER_WORKER_PORT);
  zmq::connectToSocket(this->MeshStatus,meshserver::BROKER_STATUS_PORT);
  }

bool execute()
{
  while(true)
    {
    std::string msg = zmq::s_recv(this->MeshJobs);
    std::cout << "worker sent message: " << msg << std::endl;
    //Send results back to the sink
    zmq::s_send(this->MeshStatus,msg);
    }

  return true;
}

private:
  zmq::context_t Context;
  zmq::socket_t MeshJobs;
  zmq::socket_t MeshStatus;
};
}


#endif
