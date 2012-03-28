/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __client_h
#define __client_h

#include <zmq.hpp>
#include <string>

#include <meshserver/internal/meshServerGlobals.h>
#include <meshserver/internal/JobMessage.h>
#include <meshserver/internal/JobResponse.h>
#include <meshserver/internal/zmqHelper.h>

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
  meshserver::STATUS_TYPE jobStatus(meshserver::MESH_TYPE mtype,
                                    const std::string& jobId);

  //Return the path of a completed mesh job. Will return an empty string
  //if the mesh is not completed
  std::string retrieveMesh(meshserver::MESH_TYPE mtype,
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
  meshserver::internal::JobMessage j(mtype,meshserver::CAN_MESH);
  j.send(this->Server);

  meshserver::internal::JobResponse response(this->Server);
  return response.dataAs<meshserver::STATUS_TYPE>() != meshserver::INVALID;
}

//------------------------------------------------------------------------------
std::string Client::submitMeshJob(meshserver::MESH_TYPE mtype, const std::string& fpath)
{
  meshserver::internal::JobMessage j(mtype,
                           meshserver::MAKE_MESH,
                           fpath.data(),
                           fpath.size());
  j.send(this->Server);

  meshserver::internal::JobResponse response(this->Server);
  return response.dataAs<std::string>();
}

//------------------------------------------------------------------------------
meshserver::STATUS_TYPE Client::jobStatus(meshserver::MESH_TYPE mtype, const std::string& job)
{
  meshserver::internal::JobMessage j(mtype,
                           meshserver::MESH_STATUS,
                           job.data(), job.size());
  j.send(this->Server);

  meshserver::internal::JobResponse response(this->Server);
  return response.dataAs<meshserver::STATUS_TYPE>();
}

//------------------------------------------------------------------------------
std::string Client::retrieveMesh(meshserver::MESH_TYPE mtype, const std::string& jobId)
{
  meshserver::internal::JobMessage j(mtype,
                           meshserver::MAKE_MESH,
                           jobId.data(),
                           jobId.size());
  j.send(this->Server);

  meshserver::internal::JobResponse response(this->Server);
  return response.dataAs<std::string>();
}

}


#endif
