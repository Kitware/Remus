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

  //needs its own status table of keys
  //so job:<strings>
  //and worker:<strings>
  //so that people can easily build sub listeners

  //tell this EventPublisher  what socket to send all information out on.
  //the socket_t is merely used, not owned by this class so it's lifespan
  //must be externally managed.
  bool socketToUse( zmq::socket_t* s );

  void jobQueued( const remus::proto::Job& j );
  void jobStatus( const remus::proto::JobStatus& s,
                  const zmq::SocketIdentity &workerIdentity);
  void jobTerminated( const remus::proto::JobStatus& last_status,
                      const zmq::SocketIdentity &workerIdentity);
  void jobTerminated( const remus::proto::JobStatus& last_status );

  void jobExpired( const remus::proto::JobStatus& expired_status );

  //helper method
  void jobExpired( const std::vector<remus::proto::JobStatus>& expired_status )
    {
      typedef std::vector<remus::proto::JobStatus>::const_iterator it;
      for(it i = expired_status.begin(); i != expired_status.end(); ++i)
        {
        this->jobExpired( *i );
        }
    }

  void jobFinished( const remus::proto::JobResult& r,
                    const zmq::SocketIdentity &workerIdentity);
  void jobSentToWorker( const remus::worker::Job& j,
                       const zmq::SocketIdentity &workerIdentity);

  void workerReady(const zmq::SocketIdentity &workerIdentity);
  void workerRegistered(const zmq::SocketIdentity &workerIdentity);
  void workerHeartbeat(const zmq::SocketIdentity &workerIdentity);
  void workerTerminate(const zmq::SocketIdentity &workerIdentity);

  void error(const std::string& msg);

  void stop();

private:
  void pubJob(const std::string& st, const std::string suid, cJSON *root);
  void pubWorker(const std::string& st, const std::string suid, cJSON *root);

  zmq::socket_t* socket;
  std::stringstream buffer;
};

}
}
}

#endif
