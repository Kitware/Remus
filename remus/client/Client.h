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

#include <remus/client/Job.h>
#include <remus/client/JobResult.h>
#include <remus/client/JobRequest.h>
#include <remus/client/JobStatus.h>

#include <remus/common/Message.h>
#include <remus/common/Response.h>
#include <remus/common/zmqHelper.h>
#include <remus/client/ServerConnection.h>

//included for symbol exports
#include <remus/client/ClientExports.h>

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

}

//We want the user to have a nicer experience creating the client interface.
//For this reason we remove the stuttering when making an instance of the client.
typedef remus::client::Client Client;

}

#endif
