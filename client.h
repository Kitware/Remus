/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __client_h
#define __client_h

#include <zmq.h>
#include "Common/meshServerGlobals.h"
#include "Common/zmqHelper.h"
#include "Common/job.h"

#include <sstream>
#include <iostream>

namespace meshserver
{
class Client
{
public:

Client():
  Context(1),
  Server(this->Context, ZMQ_DEALER)
  {
  zmq::connectToSocket(this->Server,meshserver::BROKER_CLIENT_PORT);
  }

bool execute()
{
  for(int i=0; i < 10; i++)
    {
    std::string jobData("test contents");
    meshserver::jobMessage j(meshserver::MESH2D,
                             meshserver::MAKE_MESH,
                             i,
                             jobData.data(),
                             jobData.size());
    j.send(this->Server);
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
