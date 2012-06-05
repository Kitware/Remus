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

#ifndef __remus_client_h
#define __remus_client_h

#include <zmq.hpp>
#include <string>

#include <remus/JobResult.h>
#include <remus/JobStatus.h>
#include <remus/common/JobMessage.h>
#include <remus/common/JobResponse.h>
#include <remus/common/meshServerGlobals.h>
#include <remus/common/zmqHelper.h>
#include <remus/client/ServerConnection.h>


namespace remus{
namespace client{
class Client
{
public:
  //connect to a given host on a given port with tcp
  explicit Client(const remus::client::ServerConnection& conn);

  //Submit a request to the server to see if it support the type of mesh
  bool canMesh(remus::MESH_TYPE mtype);

  //Submit a mesh job to the server. Currently only supports submitting
  //text/file path based jobs. Returns the job unique id as a string. Failure
  //to submit the job is handled by return an empty string
  std::string submitMeshJob(remus::MESH_TYPE mtype,
                            const std::string& fpath);

  //Given a mesh type and the job unique id as a string will return the
  //status of the job.
  remus::JobStatus jobStatus(remus::MESH_TYPE mtype,
                                    const std::string& jobId);

  //Return the path of a completed mesh job. Will return an empty string
  //if the mesh is not completed
  remus::JobResult retrieveMesh(remus::MESH_TYPE mtype,
                           const std::string& jobId);

private:
  zmq::context_t Context;
  zmq::socket_t Server;
};

//------------------------------------------------------------------------------
Client::Client(const remus::client::ServerConnection &conn):
  Context(1),
  Server(this->Context, ZMQ_REQ)
{
  zmq::connectToAddress(this->Server,conn.endpoint());
}

//------------------------------------------------------------------------------
bool Client::canMesh(remus::MESH_TYPE mtype)
{
  remus::common::JobMessage j(mtype,remus::CAN_MESH);
  j.send(this->Server);

  remus::common::JobResponse response(this->Server);
  return response.dataAs<remus::STATUS_TYPE>() != remus::INVALID_STATUS;
}

//------------------------------------------------------------------------------
std::string Client::submitMeshJob(remus::MESH_TYPE mtype, const std::string& fpath)
{
  remus::common::JobMessage j(mtype,
                           remus::MAKE_MESH,
                           fpath.data(),
                           fpath.size());
  j.send(this->Server);

  remus::common::JobResponse response(this->Server);
  return response.dataAs<std::string>();
}

//------------------------------------------------------------------------------
remus::JobStatus Client::jobStatus(remus::MESH_TYPE mtype, const std::string& job)
{
  remus::common::JobMessage j(mtype,
                           remus::MESH_STATUS,
                           job.data(), job.size());
  j.send(this->Server);

  remus::common::JobResponse response(this->Server);
  const std::string status = response.dataAs<std::string>();
  return remus::to_JobStatus(status);
}

//------------------------------------------------------------------------------
remus::JobResult Client::retrieveMesh(remus::MESH_TYPE mtype, const std::string& jobId)
{
  remus::common::JobMessage j(mtype,
                           remus::MAKE_MESH,
                           jobId.data(),
                           jobId.size());
  j.send(this->Server);

  remus::common::JobResponse response(this->Server);
  const std::string result = response.dataAs<std::string>();
  return remus::to_JobResult(result);
}

}
}

#endif
