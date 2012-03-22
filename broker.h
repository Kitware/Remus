/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __server_h
#define __server_h

#include <iostream>
#include <sstream>

#include <zmq.h>
#include "Common/meshServerGlobals.h"
#include "Common/job.h"
#include "Common/zmqHelper.h"

namespace meshserver
{
class Broker
{
public:
  Broker():
  Context(1),
  JobQueries(this->Context,ZMQ_REP),
  Workers(this->Context,ZMQ_PUSH),
  WorkerStatus(this->Context, ZMQ_PULL)
  {
  zmq::bindToSocket(JobQueries,meshserver::BROKER_CLIENT_PORT);
  zmq::bindToSocket(Workers,meshserver::BROKER_WORKER_PORT);
  zmq::bindToSocket(WorkerStatus,meshserver::BROKER_STATUS_PORT);
  }

bool execute()
{
  zmq::pollitem_t items[2] = {
      { this->JobQueries,  0, ZMQ_POLLIN, 0 },
      { this->WorkerStatus, 0, ZMQ_POLLIN, 0 } };

  //  Process messages from both sockets
  while (true)
    {
    zmq::poll (&items[0], 2, -1);
    if (items [0].revents & ZMQ_POLLIN)
      {
      this->ProcessJobQuery();
      }
    if (items [1].revents & ZMQ_POLLIN)
      {
      this->ProcessWorkerStatus();
      }
    }
  return true;
  }

private:

void ProcessJobQuery()
  {
  //we could have a job item or a job query
  //so make the generic class type
  //construct a job item from the latest item to be recieved from job connection
  meshserver::jobMessage jmsg(this->JobQueries);

  //we need to hold onto job and push it to a server instead
  if(jmsg.serviceType() == meshserver::MAKE_MESH)
    {
    std::cout << "got a job to make a mesh" << std::endl;
    }

}

void ProcessWorkerStatus()
{
  //no-op for now
}


  zmq::context_t Context;
  zmq::socket_t JobQueries;
  zmq::socket_t Workers;
  zmq::socket_t WorkerStatus;
};

}


#endif
