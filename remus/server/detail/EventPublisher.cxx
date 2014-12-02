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

namespace
{
//------------------------------------------------------------------------------
void json_free(void *data, void *hint)
{ //cjson memory is malloced
  free(data);
  (void)hint;
}

}


namespace remus{
namespace server{
namespace detail{

//----------------------------------------------------------------------------
bool EventPublisher::socketToUse( zmq::socket_t* s )
{
  this->socket = s;
  return true;
}

//----------------------------------------------------------------------------
void EventPublisher::post(const remus::proto::Job& j )
{ //queue job
  buffer << j.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string serv_t = remus::common::serv_types[(int)remus::QUEUED];

  buffer << j.type();
  const std::string& mesh_t = buffer.str(); buffer.str("");

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "mesh_type", cJSON_CreateString(mesh_t.c_str()));
  this->postJob(serv_t, suid, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::post(const remus::proto::JobStatus& s, const zmq::SocketIdentity &si)
{ //status
  buffer << s.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = zmq::to_string(si);

  const std::string serv_t = remus::common::serv_types[(int)remus::MESH_STATUS];
  const std::string status_t = remus::common::stat_types[(int)s.status()];

  buffer << s.progress();
  const std::string progress_t = buffer.str(); buffer.str("");

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  cJSON_AddItemToObject(root, "status_type", cJSON_CreateString(status_t.c_str()));
  cJSON_AddItemToObject(root, "progress", cJSON_CreateString(progress_t.c_str()));
  this->postJob(serv_t, suid, root);
  this->postWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::post(const remus::proto::JobResult& r, const zmq::SocketIdentity &si)
{ //have result to fetch
  buffer << r.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = zmq::to_string(si);

  const std::string serv_t = remus::common::serv_types[(int)remus::RETRIEVE_RESULT];
  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  this->postJob(serv_t, suid, root);
  this->postWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

  //----------------------------------------------------------------------------
void EventPublisher::post(const remus::worker::Job& j, const zmq::SocketIdentity &si)
{ //assign job to worker
  buffer << j.id();
  const std::string suid = buffer.str(); buffer.str("");
  const std::string work_t = zmq::to_string(si);

  const std::string serv_t = remus::common::serv_types[(int)remus::MAKE_MESH];

  cJSON *root;
  root=cJSON_CreateObject();
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(suid.c_str()));
  cJSON_AddItemToObject(root, "msg_type", cJSON_CreateString(serv_t.c_str()));
  cJSON_AddItemToObject(root, "worker_id", cJSON_CreateString(work_t.c_str()));
  this->postJob(serv_t, suid, root);
  this->postWorker(serv_t, work_t, root);

  cJSON_Delete(root);
}

//----------------------------------------------------------------------------
void EventPublisher::error(const std::string& msg)
{
  //send the key that people subscribe to

  //error key is "error:server", this is to allow in the future for
  //us to send specific component level failure messages
  static const std::string error_key = "error:server";
  socket->send(error_key.data(), error_key.size(), ZMQ_SNDMORE);

  //send the actual data of the message to publish
  socket->send(msg.data(), msg.size());
}

//----------------------------------------------------------------------------
void EventPublisher::stop()
{
  //publish
  const std::string error_key = "error";
  const std::string stop_key = "stop";
  const std::string stop_value = "END";

  //first send a message on error that the server is ending
  socket->send(error_key.data(), error_key.size(), ZMQ_SNDMORE);
  socket->send(stop_value.data(), stop_value.size());

  //now send the same message on the stop channel
  socket->send(stop_key.data(), stop_key.size(), ZMQ_SNDMORE);
  socket->send(stop_value.data(), stop_value.size());
}

//----------------------------------------------------------------------------
void EventPublisher::postJob(const std::string& st, const std::string suid, cJSON *root)
{
  //we use job/service/id over job/id/service since the latter is
  //impossible to monitor for only submit/status/queue by using zmq
  //subscription features. The form we use allows subscription to a single
  //job ( with a bit of setup ), and allows somebody to watch for all messages
  //of a given status type
  std::string key = "job:" + st + ":" + suid;
  socket->send(key.data(), key.size(), ZMQ_SNDMORE);

  //send the actual data of the message to publish
  char *json_str = cJSON_PrintUnformatted(root);
  std::size_t len = std::strlen(json_str);

  //zero copy zmq message
  void *hint = NULL;
  zmq::message_t msg(json_str, len, json_free, hint);
  socket->send(msg.data(), msg.size());
}

//----------------------------------------------------------------------------
void EventPublisher::postWorker(const std::string& st, const std::string suid, cJSON *root)
{
  //we use worker/service/id over worker/id/service since the latter is
  //impossible to monitor for only submit/status/queue by using zmq
  //subscription features. The form we use allows subscription to a single
  //job ( with a bit of setup ), and allows somebody to watch for all messages
  //of a given status type
  std::string key = "worker:" + st + ":" + suid;
  socket->send(key.data(), key.size(), ZMQ_SNDMORE);

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
