/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __server_h
#define __server_h

#include <iostream>
#include <sstream>

#include "MeshServerInfo.h"
#include "zmq.hpp"
#include "zeroHelper.h"

namespace meshserver
{
class Broker
{
public:
  Broker():
  Context(1),
  ClientRequests(this->Context,ZMQ_REP),
  Workers(this->Context,ZMQ_PUSH),
  WorkerStatus(this->Context, ZMQ_PULL)
  {
  zmq::bindToSocket(ClientRequests,meshserver::BROKER_CLIENT_PORT);
  zmq::bindToSocket(Workers,meshserver::BROKER_WORKER_PORT);
  zmq::bindToSocket(WorkerStatus,meshserver::BROKER_STATUS_PORT);
  }

bool execute()
{
  //loop forever waiting for worker status, and pushing it
  while(true)
    {
    //todo add polling which checks for mesh request
    //and mesh status and updates everything as needed
    zmq::s_recv(this->ClientRequests);
    zmq::s_send(this->Workers,"Broker Requested Mesh Job");
    zmq::s_send(this->ClientRequests, "Sent a request for meshing to workers");
    }
  return true;
}

private:
  zmq::context_t Context;
  zmq::socket_t ClientRequests;
  zmq::socket_t Workers;
  zmq::socket_t WorkerStatus;
};

}


#endif
