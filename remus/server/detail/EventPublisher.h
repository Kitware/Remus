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

#ifndef remus_server_detail_EventPublisher_h
#define remus_server_detail_EventPublisher_h

#include <remus/proto/Job.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/worker/Job.h>

#include <remus/proto/zmq.hpp>
#include "cJSON.h"

namespace remus{
namespace server{
namespace detail{

class EventPublisher
{
public:
  EventPublisher():
    socket(NULL),
    buffer()
  {

  }

  //tell this EventPublisher  what socket to send all information out on.
  //the socket_t is merely used, not owned by this class so it's lifespan
  //must be externally managed.
  bool socketToUse( zmq::socket_t* s );

  void post( const remus::proto::Job& j );
  void post( const remus::proto::JobStatus& s,
             const zmq::SocketIdentity &workerIdentity);
  void post( const remus::proto::JobResult& r,
             const zmq::SocketIdentity &workerIdentity);
  void post( const remus::worker::Job& j,
             const zmq::SocketIdentity &workerIdentity);

  void error(const std::string& msg);

  void stop();

private:
  void postJob(const std::string& st, const std::string suid, cJSON *root);
  void postWorker(const std::string& st, const std::string suid, cJSON *root);

  zmq::socket_t* socket;
  std::stringstream buffer;
};

}
}
}

#endif
