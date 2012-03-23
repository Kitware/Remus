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
class JobMessage;
class Broker
{
public:
  Broker();
  bool startBrokering();

private:
  //processes all job queries
  void DetermineJobResponse(const std::string &clientAddress,
                            meshserver::JobMessage* jmsg);

  //These methods are all to do with send responses to job messages
  bool canMesh(meshserver::JobMessage* msg);
  meshserver::STATUS_TYPE meshStatus(meshserver::JobMessage* msg);
  std::string queueJob(meshserver::JobMessage* msg);
  std::string retrieveMesh(meshserver::JobMessage* msg);


  //Methods for processing Worker queries
  void DetermineWorkerResponse();



  zmq::context_t Context;
  zmq::socket_t JobQueries;
  zmq::socket_t WorkerQueries;
  zmq::socket_t Workers;
};

}


#endif
