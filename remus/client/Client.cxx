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

#include <remus/common/Message.h>
#include <remus/common/Response.h>

namespace remus{
namespace client{

//------------------------------------------------------------------------------
Client::Client(const remus::client::ServerConnection &conn):
  Context(1),
  Server(Context, ZMQ_REQ),
  ConnectedToLocalServer( conn.isLocalEndpoint() )
{
  zmq::connectToAddress(this->Server,conn.endpoint());
}

//------------------------------------------------------------------------------
bool Client::canMesh(const remus::common::MeshIOType& meshtypes)
{
  remus::common::Message j(meshtypes, remus::CAN_MESH);
  j.send(this->Server);

  remus::common::Response response(this->Server);
  return response.dataAs<remus::STATUS_TYPE>() != remus::INVALID_STATUS;
}

//------------------------------------------------------------------------------
std::set < remus::client::JobMeshRequirements >
Client::retrieveMeshRequirements( const remus::common::MeshIOType& meshtypes)
{
  remus::common::Message j(meshtypes, remus::MESH_REQUIREMENTS);
  j.send(this->Server);

  remus::common::Response response(this->Server);

  std::istringstream buffer(response.dataAs<std::string>());

  JobMeshRequirementsSet set;
  buffer >> set;
  return set.get(); //copy on return on purpose
}

//------------------------------------------------------------------------------
remus::client::Job
Client::submitJob(const remus::client::JobSubmission& submission)
{
  //hold as a string so message doesn't have to copy a second time
  const std::string stringRequest(remus::client::to_string(submission));
  remus::common::Message j(submission.type(),
                           remus::MAKE_MESH,
                           stringRequest.data(),
                           stringRequest.size());
  j.send(this->Server);

  remus::common::Response response(this->Server);
  const std::string job = response.dataAs<std::string>();
  return remus::client::to_Job(job);
}

//------------------------------------------------------------------------------
remus::client::JobStatus Client::jobStatus(const remus::client::Job& job)
{
  remus::common::Message j(job.type(),
                              remus::MESH_STATUS,
                              remus::client::to_string(job));
  j.send(this->Server);

  remus::common::Response response(this->Server);
  const std::string status = response.dataAs<std::string>();
  return remus::client::to_JobStatus(status);
}

//------------------------------------------------------------------------------
remus::client::JobResult Client::retrieveResults(const remus::client::Job& job)
{
  remus::common::Message j(job.type(),
                              remus::RETRIEVE_MESH,
                              remus::client::to_string(job));
  j.send(this->Server);

  remus::common::Response response(this->Server);
  const std::string result = response.dataAs<std::string>();
  return remus::client::to_JobResult(result);
}

//------------------------------------------------------------------------------
remus::client::JobStatus Client::terminate(const remus::client::Job& job)
{
  remus::common::Message j(job.type(),
                              remus::TERMINATE_JOB_AND_WORKER,
                              remus::client::to_string(job));
  j.send(this->Server);

  remus::common::Response response(this->Server);
  const std::string status = response.dataAs<std::string>();
  return remus::client::to_JobStatus(status);
}

}
}
