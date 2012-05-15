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

#ifndef __meshserver_client_h
#define __meshserver_client_h

#include <zmq.hpp>
#include <string>

#include <meshserver/JobResult.h>
#include <meshserver/JobStatus.h>
#include <meshserver/common/JobMessage.h>
#include <meshserver/common/JobResponse.h>
#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>
#include <meshserver/client/ServerConnection.h>


namespace meshserver{
namespace client{
class Client
{
public:
  //connect to a given host on a given port with tcp
  explicit Client(const meshserver::client::ServerConnection& conn);

  //Submit a request to the server to see if it support the type of mesh
  bool canMesh(meshserver::MESH_TYPE mtype);

  //Submit a mesh job to the server. Currently only supports submitting
  //text/file path based jobs. Returns the job unique id as a string. Failure
  //to submit the job is handled by return an empty string
  std::string submitMeshJob(meshserver::MESH_TYPE mtype,
                            const std::string& fpath);

  //Given a mesh type and the job unique id as a string will return the
  //status of the job.
  meshserver::JobStatus jobStatus(meshserver::MESH_TYPE mtype,
                                    const std::string& jobId);

  //Return the path of a completed mesh job. Will return an empty string
  //if the mesh is not completed
  meshserver::JobResult retrieveMesh(meshserver::MESH_TYPE mtype,
                           const std::string& jobId);

private:
  zmq::context_t Context;
  zmq::socket_t Server;
};

//------------------------------------------------------------------------------
Client::Client(const meshserver::client::ServerConnection &conn):
  Context(1),
  Server(this->Context, ZMQ_REQ)
{
  zmq::connectToAddress(this->Server,conn.endpoint());
}

//------------------------------------------------------------------------------
bool Client::canMesh(meshserver::MESH_TYPE mtype)
{
  meshserver::common::JobMessage j(mtype,meshserver::CAN_MESH);
  j.send(this->Server);

  meshserver::common::JobResponse response(this->Server);
  return response.dataAs<meshserver::STATUS_TYPE>() != meshserver::INVALID_STATUS;
}

//------------------------------------------------------------------------------
std::string Client::submitMeshJob(meshserver::MESH_TYPE mtype, const std::string& fpath)
{
  meshserver::common::JobMessage j(mtype,
                           meshserver::MAKE_MESH,
                           fpath.data(),
                           fpath.size());
  j.send(this->Server);

  meshserver::common::JobResponse response(this->Server);
  return response.dataAs<std::string>();
}

//------------------------------------------------------------------------------
meshserver::JobStatus Client::jobStatus(meshserver::MESH_TYPE mtype, const std::string& job)
{
  meshserver::common::JobMessage j(mtype,
                           meshserver::MESH_STATUS,
                           job.data(), job.size());
  j.send(this->Server);

  meshserver::common::JobResponse response(this->Server);
  const std::string status = response.dataAs<std::string>();
  return meshserver::to_JobStatus(status);
}

//------------------------------------------------------------------------------
meshserver::JobResult Client::retrieveMesh(meshserver::MESH_TYPE mtype, const std::string& jobId)
{
  meshserver::common::JobMessage j(mtype,
                           meshserver::MAKE_MESH,
                           jobId.data(),
                           jobId.size());
  j.send(this->Server);

  meshserver::common::JobResponse response(this->Server);
  const std::string result = response.dataAs<std::string>();
  return meshserver::to_JobResult(result);
}

}
}

#endif
