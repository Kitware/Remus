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
#include <set>

#include <boost/scoped_ptr.hpp>

#include <remus/client/ServerConnection.h>

#include <remus/proto/Job.h>
#include <remus/proto/JobRequirements.h>
#include <remus/proto/JobResult.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/JobSubmission.h>

//included for export symbols
#include <remus/client/ClientExports.h>

//The client class is used to submit meshing jobs to a remus server.
//The class also allows you to query on the state of a given job and
//to retrieve the results of the job when it is finished.
namespace remus{
namespace client{

namespace detail { struct ZmqManagement; }

class REMUSCLIENT_EXPORT Client
{
public:
  //connect to a given host on a given port with tcp
  explicit Client(const remus::client::ServerConnection& conn);

  //explicit destructor since we use the pimpl idiom, and
  //forward declare ZmqManagement so the destructor needs
  //to know the full type information to properly delete it
  ~Client();

  //return the connection info that was used to connect to the
  //remus server
  const remus::client::ServerConnection& connection() const;

  //Submit a request to the server to see if the server supports
  //the requested input and output mesh types
  bool canMesh(const remus::common::MeshIOType& meshtypes);

  //Submit a request to the server to see if the server supports
  //the exact requested requirements
  bool canMesh(const remus::proto::JobRequirements& requirements);

  //submit a request to the server to see if the server supports
  //the request input and output mesh types. If the server does support
  //the given types, return a collection of JobRequirements
  remus::proto::JobRequirementsSet
  retrieveRequirements( const remus::common::MeshIOType& meshtypes );

  //Submit a job to the server. The job submission has a JobData and
  //a JobRequirements component
  remus::proto::Job submitJob(const remus::proto::JobSubmission& submission);

  //Given a remus Job object returns the status of the job
  remus::proto::JobStatus jobStatus(const remus::proto::Job& job);

  //Return job result of of a give job
  remus::proto::JobResult retrieveResults(const remus::proto::Job& job);

  //attempts to terminate a given job, will kill the job if the job hasn't
  //started. If the job has been finished and the results
  //are on the server the results will be deleted. If the job is in process
  //this will be unable to kill the job.
  remus::proto::JobStatus terminate(const remus::proto::Job& job);

protected:
  remus::client::ServerConnection ConnectionInfo;
private:
  //explicitly state the client doesn't support copy or move semantics
  Client(const Client&);
  void operator=(const Client&);

  boost::scoped_ptr<detail::ZmqManagement> Zmq;
};

}

//We want the user to have a nicer experience creating the client interface.
//For this reason we remove the stuttering when making an instance of the client.
typedef remus::client::Client Client;

}

#endif
