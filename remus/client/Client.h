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

#ifndef remus_client_Client_h
#define remus_client_Client_h

#include <string>
#include <vector>

#include <remus/client/Job.h>
#include <remus/client/JobResult.h>
#include <remus/client/JobRequest.h>
#include <remus/client/JobStatus.h>

#include <remus/common/Message.h>
#include <remus/common/Response.h>
#include <remus/common/zmqHelper.h>
#include <remus/client/ServerConnection.h>


//The client class is used to submit meshing jobs to a remus server.
//The class also allows you to query on the state of a given job and
//to retrieve the results of the job when it is finished.
namespace remus{
namespace client{
class Client
{
public:
  //connect to a given host on a given port with tcp
  explicit Client(const remus::client::ServerConnection& conn);

  //Submit a request to the server to see if it support the requirements
  //of a given job request
  bool canMesh(const remus::client::JobRequest& request);

  //Submit a job to the server.
  remus::client::Job submitJob(const remus::client::JobRequest& request);

  //Given a remus Job object returns the status of the job
  remus::client::JobStatus jobStatus(const remus::client::Job& job);

  //Return job result of of a give job
  remus::client::JobResult retrieveResults(const remus::client::Job& job);

  //attempts to terminate a given job, will kill the worker of a job
  //if the job is still pending. If the job has been finished and the results
  //are on the server the results will be deleted.
  remus::client::JobStatus terminate(const remus::client::Job& job);

private:
  zmq::context_t Context;
  zmq::socket_t Server;
};

//------------------------------------------------------------------------------
Client::Client(const remus::client::ServerConnection &conn):
  Context(1),
  Server(Context, ZMQ_REQ)
{
  zmq::connectToAddress(this->Server,conn.endpoint());
}

//------------------------------------------------------------------------------
bool Client::canMesh(const remus::client::JobRequest& request)
{
  //hold as a string so message doesn't have to copy a second time
  const std::string stringRequest(remus::client::to_string(request));
  remus::common::Message j(request.type(),
                              remus::CAN_MESH,
                              stringRequest.data(),
                              stringRequest.size());
  j.send(this->Server);

  remus::common::Response response(this->Server);
  return response.dataAs<remus::STATUS_TYPE>() != remus::INVALID_STATUS;
}

//------------------------------------------------------------------------------
remus::client::Job Client::submitJob(const remus::client::JobRequest& request)
{
  //hold as a string so message doesn't have to copy a second time
  const std::string stringRequest(remus::client::to_string(request));
  remus::common::Message j(request.type(),
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

//We want the user to have a nicer experience creating the client interface.
//For this reason we remove the stuttering when making an instance of the client.
typedef remus::client::Client Client;
}

#endif
