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

#include <remus/client/Client.h>

#include <remus/proto/Message.h>
#include <remus/proto/Response.h>

namespace remus{
namespace client{

//lightweight struct to hide zmq from leaking into libraries that link
//to remus client
namespace detail{
struct ZmqManagement
{
  zmq::context_t Context;
  zmq::socket_t Server;

  ZmqManagement():
    Context(1),
    Server(Context, ZMQ_REQ)
  {}
};
}

//------------------------------------------------------------------------------
Client::Client(const remus::client::ServerConnection &conn):
  ConnectionInfo(conn),
  Zmq( new detail::ZmqManagement() )
{
  zmq::connectToAddress(this->Zmq->Server,conn.endpoint());
}

//------------------------------------------------------------------------------
Client::~Client()
{
}

//------------------------------------------------------------------------------
const remus::client::ServerConnection& Client::connection() const
{
  return this->ConnectionInfo;
}

//------------------------------------------------------------------------------
bool Client::canMesh(const remus::common::MeshIOType& meshtypes)
{
  remus::proto::Message j(meshtypes, remus::CAN_MESH);
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);
  return response.dataAs<remus::STATUS_TYPE>() != remus::INVALID_STATUS;
}

//------------------------------------------------------------------------------
bool Client::canMesh(const remus::proto::JobRequirements& reqs)
{
  const std::string stringRequest(remus::proto::to_string(reqs));
  remus::proto::Message j(reqs.meshTypes(),
                          remus::CAN_MESH_REQUIREMENTS,
                          stringRequest);
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);
  return response.dataAs<remus::STATUS_TYPE>() != remus::INVALID_STATUS;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet
Client::retrieveRequirements( const remus::common::MeshIOType& meshtypes)
{
  remus::proto::Message j(meshtypes, remus::MESH_REQUIREMENTS);
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);

  std::istringstream buffer(response.dataAs<std::string>());

  remus::proto::JobRequirementsSet set;
  buffer >> set;
  return set;
}

//------------------------------------------------------------------------------
remus::proto::Job
Client::submitJob(const remus::proto::JobSubmission& submission)
{
  //hold as a string so message doesn't have to copy a second time
  const std::string stringRequest(remus::proto::to_string(submission));
  remus::proto::Message j(submission.type(),
                           remus::MAKE_MESH,
                           stringRequest.data(),
                           stringRequest.size());
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);
  const std::string job = response.dataAs<std::string>();
  return remus::proto::to_Job(job);
}

//------------------------------------------------------------------------------
remus::proto::JobStatus Client::jobStatus(const remus::proto::Job& job)
{
  remus::proto::Message j(job.type(),
                              remus::MESH_STATUS,
                              remus::proto::to_string(job));
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);
  const std::string status = response.dataAs<std::string>();
  return remus::proto::to_JobStatus(status);
}

//------------------------------------------------------------------------------
remus::proto::JobResult Client::retrieveResults(const remus::proto::Job& job)
{
  remus::proto::Message j(job.type(),
                              remus::RETRIEVE_MESH,
                              remus::proto::to_string(job));
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);
  const std::string result = response.dataAs<std::string>();
  return remus::proto::to_JobResult(result);
}

//------------------------------------------------------------------------------
remus::proto::JobStatus Client::terminate(const remus::proto::Job& job)
{
  remus::proto::Message j(job.type(),
                              remus::TERMINATE_JOB,
                              remus::proto::to_string(job));
  j.send(this->Zmq->Server);

  remus::proto::Response response(this->Zmq->Server);
  const std::string status = response.dataAs<std::string>();
  return remus::proto::to_JobStatus(status);
}

}
}
