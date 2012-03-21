/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __worker_h
#define __worker_h

#include "MeshServerInfo.h"
#include "zmq.hpp"
#include "zeroHelper.h"

#include <sstream>
#include <iostream>

namespace meshserver
{
class Client
{
public:

Client():
  Context(1),
  Server(this->Context, ZMQ_REQ)
  {
  zmq::connectToSocket(this->Server,meshserver::BROKER_CLIENT_PORT);
  }

bool execute()
{
  for(int i=0; i < 10; i++)
    {
    zmq::s_send(this->Server, "I want a mesh job!");
    std::cout << "Client recieved msg: " << zmq::s_recv(this->Server) << std::endl;
    }

  this->Server.close();
  return true;
}

private:
  zmq::context_t Context;
  zmq::socket_t Server;
};
}


#endif
