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


//forward declares for all the remus class we use
namespace remus{

namespace proto{
  class Job;
  class JobStatus;
  class JobResult;
  }

namespace worker{ class Job;   }
}

namespace zmq
{
  struct SocketIdentity;
}

//forward declare the cjson struct
struct cJSON;

#include <remus/proto/zmq.hpp>

#include <sstream>
#include <vector>
#include <set>


namespace remus{
namespace server{
namespace detail{


//Todo: We need to make this class waaaay faster
//it does way to much json creation and deletion, we need to cache
//those tables as much as possible on construction

//We need to create look-ups for workerIdentities so we don't have
//to hash those every time. Preferably the SocketIdentity will get that
//ability
class EventPublisher
{
public:
  EventPublisher():
    socket(NULL),
    buffer()
  {

  }

  //Job status sections
  //QUEUED
  //MESH_STATUS
  //TERMINATE_JOB
  //EXPIRED
  //COMPLETED
  //ASSIGNED_TO_WORKER

  //Worker status sections
  //REGISTERED
  //ASKING_FOR_JOB
  //HEARTBEAT
  //WORKER_STATE
  //TERMINATE_WORKER

  //The event publisher uses a multiple section key
  //to allow listeners easy control over what they want to register for
  //the key construction is:
  //job:<status>:<jobId>
  //worker:<status>:<workerId>

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

  void jobFinished( const remus::proto::JobResult& r,
                    const zmq::SocketIdentity &workerIdentity);
  void jobSentToWorker( const remus::worker::Job& j,
                       const zmq::SocketIdentity &workerIdentity);

  //helper method for when we have a collection of events to publish
  void jobsExpired( const std::vector<remus::proto::JobStatus>& expired_status );


  void workerReady(const zmq::SocketIdentity &workerIdentity);
  void workerRegistered(const zmq::SocketIdentity &workerIdentity);
  void workerHeartbeat(const zmq::SocketIdentity &workerIdentity);
  void workerResponsive(const zmq::SocketIdentity &workerIdentity);
  void workerUnresponsive(const zmq::SocketIdentity &workerIdentity);
  void workerTerminated(const zmq::SocketIdentity &workerIdentity);

  //helper method for when we have a collection of events to publish
  void unresponsiveWorkers( const std::set< zmq::SocketIdentity >& workers );

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
