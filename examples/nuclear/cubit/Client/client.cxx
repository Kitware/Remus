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

#include "client.h"

client::client(std::string server)
{
  if ( !server.empty())
    {
    Connection = remus::client::make_ServerConnection(server);
    }
}

CubitOutput client::getOutput(CubitInput const& in)
{
  remus::meshtypes::Model in_type;
  remus::meshtypes::Mesh3D out_type;

  remus::Client c(Connection);

  remus::common::MeshIOType mesh_types(in_type,out_type);
  if(c.canMesh(mesh_types))
    {
    remus::proto::JobRequirements reqs =
      remus::proto::make_MemoryJobRequirements(mesh_types,"CubitWorker","");
    remus::proto::JobContent content =
      remus::proto::make_MemoryJobContent(in);
    remus::proto::JobSubmission sub(reqs,content);

    remus::proto::Job job = c.submitJob(sub);
    remus::proto::JobStatus jobState = c.jobStatus(job);

    //wait while the job is running
    while(jobState.good())
      {
      jobState = c.jobStatus(job);
      };

    if(jobState.finished())
      {
      remus::proto::JobResult result = c.retrieveResults(job);
      return CubitOutput(result);
      }
    }
  return CubitOutput();
}
