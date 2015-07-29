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

#include <remus/server/detail/EventPublisher.h>

#include <remus/proto/EventTypes.h>
#include <remus/proto/Job.h>
#include <remus/proto/JobRequirements.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/worker/Job.h>

#include "cJSON.h"

namespace
{
//------------------------------------------------------------------------------
void json_free(void *data, void *hint)
{ //cjson memory is malloced
  free(data);
  (void)hint;
}

//------------------------------------------------------------------------------
template <typename T, typename BufferType>
std::string as_string(const T& t, BufferType& bt )
{
  bt << t;
  const std::string result = bt.str();
  bt.str("");
  return result;
}

}


namespace remus{
namespace server{
namespace detail{

//----------------------------------------------------------------------------
EventPublisher::EventPublisher():
  socket(NULL),
  buffer()
  {

  }

//----------------------------------------------------------------------------
bool EventPublisher::socketToUse( zmq::socket_t* s )
{
  this->socket = s;
  return true;
}

//----------------------------------------------------------------------------
void EventPublisher::jobQueued(const remus::proto::Job& j,
                                                        const remus::proto::JobRequirements& reqs)
{ //queue job
  const std::string suid = as_string(j.id(), buffer);
  const std::string serv_t = remus::proto::jobevents::event_types[ remus::proto::jobevents::QUEUED ];
  const std::string work_t = ""; //done for easier client parsing

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str())); //kept for easier client parsing

  const std::string source_type = as_string(reqs.sourceType(), buffer);
  const std::string format_type = as_string(reqs.formatType(), buffer);
  const std::string mesh_type = as_string(reqs.meshTypes(), buffer);
  const std::string worker_name = as_string(reqs.workerName(), buffer);
  const std::string tag = as_string(reqs.tag(), buffer);

  cJSON *jsonReqs;
  cJSON_AddItemToObject(root, "requirements", jsonReqs=cJSON_CreateObject());
  cJSON_AddItemToObject(jsonReqs, "source_type",   cJSON_CreateString(source_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "format_type",   cJSON_CreateString(format_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "mesh_type",   cJSON_CreateString(mesh_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "worker_name",   cJSON_CreateString(worker_name.c_str()));
  cJSON_AddItemToObject(jsonReqs, "tag",   cJSON_CreateString(tag.c_str()));


  this->pubJob(serv_t, suid, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::jobStatus(const remus::proto::JobStatus& s, const zmq::SocketIdentity &si)
{ //status
  buffer << s.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = si.name();

  const std::string serv_t = remus::proto::jobevents::event_types[ remus::proto::jobevents::JOB_STATUS ];
  const std::string status_t = remus::common::stat_types[(int)s.status()];

  buffer << s.progress();
  const std::string progress_t = buffer.str(); buffer.str("");

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "status_type", cJSON_CreateString(status_t.c_str()));
  cJSON_AddItemToObject(root, "progress", cJSON_CreateString(progress_t.c_str()));
  this->pubJob(serv_t, suid, root);


  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::jobTerminated(const remus::proto::JobStatus& s,
                                   const zmq::SocketIdentity &si)
{ //job assigned to a worker has been terminated
  buffer << s.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = si.name();

  const std::string serv_t = remus::proto::jobevents::event_types[ remus::proto::jobevents::TERMINATED ];
  const std::string status_t = remus::common::stat_types[(int)s.status()];

  buffer << s.progress();
  const std::string progress_t = buffer.str(); buffer.str("");

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "last_status_type", cJSON_CreateString(status_t.c_str()));
  cJSON_AddItemToObject(root, "last_progress", cJSON_CreateString(progress_t.c_str()));
  this->pubJob(serv_t, suid, root);
  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::jobTerminated(const remus::proto::JobStatus& s)
{ //job queued has been terminated
  buffer << s.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = ""; //kept for easier parsing of the json message

  const std::string serv_t = remus::proto::jobevents::event_types[ remus::proto::jobevents::TERMINATED ];
  const std::string status_t = remus::common::stat_types[(int)s.status()];

  buffer << s.progress();
  const std::string progress_t = buffer.str(); buffer.str("");

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str())); //kept for easier client parsing
  cJSON_AddItemToObject(root, "last_status_type", cJSON_CreateString(status_t.c_str()));
  cJSON_AddItemToObject(root, "last_progress", cJSON_CreateString(progress_t.c_str()));
  this->pubJob(serv_t, suid, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::jobExpired(const remus::proto::JobStatus& s)
{ //active job has been marked as expired as the worker it was assigned to
  //has stopped heartbeating
  buffer << s.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = ""; //kept for easier parsing of the json message

  const std::string serv_t = remus::proto::jobevents::event_types[ remus::proto::jobevents::EXPIRED ];
  const std::string status_t = remus::common::stat_types[(int)s.status()];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str())); //kept for easier client parsing
  this->pubJob(serv_t, suid, root);

  //now that we have published it on jobs/EXPIRED we need to also publish it
  //on jobs/STATUS
  const std::string status_service = remus::proto::jobevents::event_types[ remus::proto::jobevents::JOB_STATUS ];

  buffer << s.progress();
  const std::string progress_t = buffer.str(); buffer.str("");

  cJSON_AddItemToObject(root, "status_type", cJSON_CreateString(status_t.c_str()));
  cJSON_AddItemToObject(root, "progress", cJSON_CreateString(progress_t.c_str()));
  this->pubJob(status_service, suid, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::jobFinished(const remus::proto::JobResult& r, const zmq::SocketIdentity &si)
{ //have result to fetch
  buffer << r.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = si.name();

  const std::string serv_t = remus::proto::jobevents::event_types[ remus::proto::jobevents::COMPLETED ];
  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  this->pubJob(serv_t, suid, root);
  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

  //----------------------------------------------------------------------------
void EventPublisher::jobSentToWorker(const remus::worker::Job& j, const zmq::SocketIdentity &si)
{ //assign job to worker
  buffer << j.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = si.name();

  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::ASSIGNED_TO_WORKER ];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "job_id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  this->pubJob(serv_t, suid, root);
  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::jobsExpired( const std::vector<remus::proto::JobStatus>& expired_status )
  {
    typedef std::vector<remus::proto::JobStatus>::const_iterator it;
    for(it i = expired_status.begin(); i != expired_status.end(); ++i)
      {
      this->jobExpired( *i );
      }
  }

//----------------------------------------------------------------------------
void EventPublisher::workerReady(const zmq::SocketIdentity &workerIdentity,
                                                           const remus::proto::JobRequirements& reqs)
{
  const std::string work_t = workerIdentity.name();
  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::REGISTERED ];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));


  const std::string source_type = as_string(reqs.sourceType(), buffer);
  const std::string format_type = as_string(reqs.formatType(), buffer);
  const std::string mesh_type = as_string(reqs.meshTypes(), buffer);
  const std::string worker_name = as_string(reqs.workerName(), buffer);
  const std::string tag = as_string(reqs.tag(), buffer);

  cJSON *jsonReqs;
  cJSON_AddItemToObject(root, "requirements", jsonReqs=cJSON_CreateObject());
  cJSON_AddItemToObject(jsonReqs, "source_type",   cJSON_CreateString(source_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "format_type",   cJSON_CreateString(format_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "mesh_type",   cJSON_CreateString(mesh_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "worker_name",   cJSON_CreateString(worker_name.c_str()));
  cJSON_AddItemToObject(jsonReqs, "tag",   cJSON_CreateString(tag.c_str()));


  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::workerRegistered(const zmq::SocketIdentity &workerIdentity,
                                                                  const remus::proto::JobRequirements& reqs)
{
  const std::string work_t = workerIdentity.name();
  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::ASKING_FOR_JOB ];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));


  const std::string source_type = as_string(reqs.sourceType(), buffer);
  const std::string format_type = as_string(reqs.formatType(), buffer);
  const std::string mesh_type = as_string(reqs.meshTypes(), buffer);
  const std::string worker_name = as_string(reqs.workerName(), buffer);
  const std::string tag = as_string(reqs.tag(), buffer);

  cJSON *jsonReqs;
  cJSON_AddItemToObject(root, "requirements", jsonReqs=cJSON_CreateObject());
  cJSON_AddItemToObject(jsonReqs, "source_type",   cJSON_CreateString(source_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "format_type",   cJSON_CreateString(format_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "mesh_type",   cJSON_CreateString(mesh_type.c_str()));
  cJSON_AddItemToObject(jsonReqs, "worker_name",   cJSON_CreateString(worker_name.c_str()));
  cJSON_AddItemToObject(jsonReqs, "tag",   cJSON_CreateString(tag.c_str()));

  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::workerHeartbeat(const zmq::SocketIdentity &workerIdentity)
{
  const std::string work_t = workerIdentity.name();
  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::HEARTBEAT ];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));

  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}


//----------------------------------------------------------------------------
void EventPublisher::workerResponsive(const zmq::SocketIdentity &workerIdentity)
{
  const std::string work_t = workerIdentity.name();
  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::WORKER_STATE ];
  const std::string state_t =  "Responsive";

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "state", cJSON_CreateString(state_t.c_str()));
  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::workerUnresponsive(const zmq::SocketIdentity &workerIdentity)
{
  const std::string work_t = workerIdentity.name();
  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::WORKER_STATE ];
  const std::string state_t =  "Unresponsive";

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "state", cJSON_CreateString(state_t.c_str()));
  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::workerTerminated(const zmq::SocketIdentity &workerIdentity)
{
  const std::string work_t = workerIdentity.name();

  {
  const std::string serv_t = remus::common::serv_types[(int)remus::TERMINATE_WORKER];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));

  this->pubWorker(serv_t, work_t, root);
  cJSON_Delete(root);
  }

  {
  const std::string serv_t = remus::proto::workevents::event_types[ remus::proto::workevents::WORKER_STATE ];
  const std::string state_t =  "Unresponsive";

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "state", cJSON_CreateString(state_t.c_str()));
  this->pubWorker(serv_t, work_t, root);

  cJSON_Delete(root);
  }
}


//----------------------------------------------------------------------------
void EventPublisher::unresponsiveWorkers( const std::set< zmq::SocketIdentity >& workers )
  {
    typedef std::set<zmq::SocketIdentity>::const_iterator it;
    for(it i = workers.begin(); i != workers.end(); ++i)
      {
      this->workerUnresponsive( *i );
      }
  }

//----------------------------------------------------------------------------
void EventPublisher::error(const std::string& msg)
{
  //send the key that people subscribe to

  //error key is "error:server", this is to allow in the future for
  //us to send specific component level failure messages
  static const std::string error_key = "error:server";
  zmq::message_t keyMsg(error_key.size());
  std::memcpy(keyMsg.data(), error_key.data(), error_key.size());
  socket->send(keyMsg, ZMQ_SNDMORE);

  zmq::message_t valueMsg(msg.size());
  std::memcpy(valueMsg.data(), msg.data(), msg.size());
  socket->send(valueMsg);
}

//----------------------------------------------------------------------------
void EventPublisher::stop()
{
  //first send a message on error that the server is ending
  this->error("END");

  //publish
  const std::string stop_key = "stop";
  const std::string stop_value = "END";

  //now send the same message on the stop channel
  zmq::message_t keyMsg(stop_key.size());
  std::memcpy(keyMsg.data(), stop_key.data(), stop_key.size());
  socket->send(keyMsg, ZMQ_SNDMORE);

  zmq::message_t valueMsg(stop_value.size());
  std::memcpy(valueMsg.data(), stop_value.data(), stop_value.size());
  socket->send(valueMsg);
}

//----------------------------------------------------------------------------
void EventPublisher::pubJob(const std::string& st, const std::string suid, cJSON *root)
{
  //we use job/service/id over job/id/service since the latter is
  //impossible to monitor for only submit/status/queue by using zmq
  //subscription features. The form we use allows subscription to a single
  //job ( with a bit of setup ), and allows somebody to watch for all messages
  //of a given status type
  std::string key = "job:" + st + ":" + suid;
  zmq::message_t keyMsg(key.size());
  std::memcpy(keyMsg.data(), key.data(), key.size());
  socket->send(keyMsg, ZMQ_SNDMORE);

  //send the actual data of the message to publish
  char *json_str = cJSON_PrintUnformatted(root);
  std::size_t len = std::strlen(json_str);

  //zero copy zmq message
  void *hint = NULL;
  zmq::message_t msg(json_str, len, json_free, hint);
  socket->send(msg.data(), msg.size());
}

//----------------------------------------------------------------------------
void EventPublisher::pubWorker(const std::string& st, const std::string suid, cJSON *root)
{
  //we use worker/service/id over worker/id/service since the latter is
  //impossible to monitor for only submit/status/queue by using zmq
  //subscription features. The form we use allows subscription to a single
  //job ( with a bit of setup ), and allows somebody to watch for all messages
  //of a given status type
  std::string key = "worker:" + st + ":" + suid;
  zmq::message_t keyMsg(key.size());
  std::memcpy(keyMsg.data(), key.data(), key.size());
  socket->send(keyMsg, ZMQ_SNDMORE);

  //send the actual data of the message to publish
  char *json_str = cJSON_PrintUnformatted(root);
  std::size_t len = std::strlen(json_str);

  //zero copy zmq message
  void *hint = NULL;
  zmq::message_t msg(json_str, len, json_free, hint);
  socket->send(msg.data(), msg.size());
}


}
}
}
