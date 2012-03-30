/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __client_h
#define __client_h

#include <zmq.hpp>
#include <string>

#include <meshserver/JobMessage.h>
#include <meshserver/JobResponse.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <meshserver/common/meshServerGlobals.h>
#include <meshserver/common/zmqHelper.h>

namespace meshserver
{
class Client
{
public:
  Client();

  //Submit a request to the broker to see if it support the type of mesh
  bool canMesh(meshserver::MESH_TYPE mtype);

  //Submit a mesh job to the server. Currently only supports submitting
  //text/file path based jobs. Returns the job unique id as a string. Failure
  //to submit the job is handled by return an empty string
  std::string submitMeshJob(meshserver::MESH_TYPE mtype,
                            const std::string& fpath);

  //Given a mesh type and the job unique id as a string will return the
  //status of the job.
  meshserver::common::JobStatus jobStatus(meshserver::MESH_TYPE mtype,
                                    const std::string& jobId);

  //Return the path of a completed mesh job. Will return an empty string
  //if the mesh is not completed
  meshserver::common:JobResult retrieveMesh(meshserver::MESH_TYPE mtype,
                           const std::string& jobId);

private:
  zmq::context_t Context;
  zmq::socket_t Server;
};

//------------------------------------------------------------------------------
Client::Client():
  Context(1),
  Server(this->Context, ZMQ_REQ)
{
  //in the future this should be a dealer, but that will require
  //handling of all requests to be asynchronous
  zmq::connectToSocket(this->Server,meshserver::BROKER_CLIENT_PORT);
}

//------------------------------------------------------------------------------
bool Client::canMesh(meshserver::MESH_TYPE mtype)
{
  meshserver::JobMessage j(mtype,meshserver::CAN_MESH);
  j.send(this->Server);

  meshserver::JobResponse response(this->Server);
  return response.dataAs<meshserver::STATUS_TYPE>() != meshserver::INVALID;
}

//------------------------------------------------------------------------------
std::string Client::submitMeshJob(meshserver::MESH_TYPE mtype, const std::string& fpath)
{
  meshserver::JobMessage j(mtype,
                           meshserver::MAKE_MESH,
                           fpath.data(),
                           fpath.size());
  j.send(this->Server);

  meshserver::JobResponse response(this->Server);
  return response.dataAs<std::string>();
}

//------------------------------------------------------------------------------
meshserver::common::JobStatus Client::jobStatus(meshserver::MESH_TYPE mtype, const std::string& job)
{
  meshserver::JobMessage j(mtype,
                           meshserver::MESH_STATUS,
                           job.data(), job.size());
  j.send(this->Server);

  meshserver::JobResponse response(this->Server);
  const std::string status = response.dataAs<std::string>();
  return meshserver::to_JobStatus(status);
}

//------------------------------------------------------------------------------
meshserver::common:JobResult Client::retrieveMesh(meshserver::MESH_TYPE mtype, const std::string& jobId)
{
  meshserver::JobMessage j(mtype,
                           meshserver::MAKE_MESH,
                           jobId.data(),
                           jobId.size());
  j.send(this->Server);

  meshserver::JobResponse response(this->Server);
  const std::string result = response.dataAs<std::string>();
  return meshserver::to_JobResult(result);
}

}


#endif
