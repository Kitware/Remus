/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __broker_h
#define __broker_h

#include <zmq.hpp>
#include "Common/meshServerGlobals.h"

namespace meshserver
{
class jobMessage;
class Broker
{
public:
  Broker();
  bool startBrokering();

private:
  //processes all job queries
  void DetermineJobResponse(meshserver::jobMessage* jmsg);

  //These methods are all to do with send responses to job messages
  bool canMesh(meshserver::jobMessage* msg);
  meshserver::STATUS_TYPE meshStatus(meshserver::jobMessage* msg);
  std::string queueJob(meshserver::jobMessage* msg);
  std::string retrieveMesh(meshserver::jobMessage* msg);


  //Methods for processing Worker queries
  void DetermineWorkerResponse();



  zmq::context_t Context;
  zmq::socket_t JobQueries;
  zmq::socket_t WorkerQueries;
  zmq::socket_t Workers;
};

}


#endif
