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

#include <remus/proto/zmqHelper.h>
#include <sstream>

namespace remus{
namespace client{

//lightweight struct to hide zmq from leaking into libraries that link
//to remus client
namespace detail{
struct ZmqManagement
{
  zmq::socket_t Server;

  ZmqManagement(const remus::client::ServerConnection &conn):
    Server(*(conn.context()), ZMQ_REQ)
  {}
};
}

//------------------------------------------------------------------------------
Client::Client(const remus::client::ServerConnection &conn):
  ConnectionInfo(conn),
  Zmq( new detail::ZmqManagement(conn) )
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
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);
  std::istringstream buffer(response.data());

  bool serverCanMesh;
  buffer >> serverCanMesh;
  return serverCanMesh;
}

//------------------------------------------------------------------------------
bool Client::canMesh(const remus::proto::JobRequirements& reqs)
{
  std::ostringstream input_buffer;
  input_buffer << reqs;
  remus::proto::Message j(reqs.meshTypes(),
                          remus::CAN_MESH_REQUIREMENTS,
                          input_buffer.str() );
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);
  std::istringstream output_buffer(response.data());

  bool serverCanMesh;
  output_buffer >> serverCanMesh;
  return serverCanMesh;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet
Client::retrieveRequirements( const remus::common::MeshIOType& meshtypes)
{
  remus::proto::Message j(meshtypes, remus::MESH_REQUIREMENTS);
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);

  std::istringstream buffer(response.data());

  remus::proto::JobRequirementsSet set;
  buffer >> set;
  return set;
}

//------------------------------------------------------------------------------
remus::proto::Job
Client::submitJob(const remus::proto::JobSubmission& submission)
{
  std::ostringstream buffer;
  buffer << submission;
  remus::proto::Message j(submission.type(),
                           remus::MAKE_MESH,
                           buffer.str());
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);
  const std::string job = response.data();
  return remus::proto::to_Job(job);
}

//------------------------------------------------------------------------------
remus::proto::JobStatus Client::jobStatus(const remus::proto::Job& job)
{
  remus::proto::Message j(job.type(),
                          remus::MESH_STATUS,
                          remus::proto::to_string(job));
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);
  const std::string status = response.data();
  return remus::proto::to_JobStatus(status);
}

//------------------------------------------------------------------------------
remus::proto::JobResult Client::retrieveResults(const remus::proto::Job& job)
{
  remus::proto::Message j(job.type(),
                          remus::RETRIEVE_MESH,
                          remus::proto::to_string(job));
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);
  const std::string result = response.data();
  return remus::proto::to_JobResult(result);
}

//------------------------------------------------------------------------------
remus::proto::JobStatus Client::terminate(const remus::proto::Job& job)
{
  remus::proto::Message j(job.type(),
                          remus::TERMINATE_JOB,
                          remus::proto::to_string(job));
  j.send(&this->Zmq->Server);

  remus::proto::Response response(&this->Zmq->Server);
  const std::string status = response.data();
  return remus::proto::to_JobStatus(status);
}

}
}
