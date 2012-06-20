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

#ifndef __remus_JobRequest_h
#define __remus_JobRequest_h

#include <string>
#include <sstream>

#include <remus/common/remusGlobals.h>

//A job request has two purposes. First it is sent to the server to determine
//if the mesh type and possible mesher name is supported. Secondly it is used
//to request a job to be run given a mesh type and an optional mesher name.

//Note: Currently no server supports the ability to restrict a mesh request
//to a single named mesher. This option is expected to be supported in the future.
namespace remus{
class JobRequest
{
public:

  //Construct a job request with only a mesher type given. This is used to
  //when asking a server if it supports a type of mesh
  JobRequest(remus::MESH_TYPE type):
    Type(type),
    JobInfo(),
    NeedsSpecificMesher(false),
    MesherName()
    {
    }

  //Construct a job request with a mesh type and the info required by the worker
  //to run the job. This is used when submitting a job from the client to the server.
  JobRequest(remus::MESH_TYPE type,
             const std::string& info):
    Type(type),
    JobInfo(info),
    NeedsSpecificMesher(false),
    MesherName()
    {
    }

  //Construct a job request with a mesh type, job data and a required mesher name.
  //This is used when you need a specific mesher only to take this job from the
  //server. Note: The feature of workers getting jobs by mesher name is currently
  //unsupported.
  JobRequest(remus::MESH_TYPE type,
             const std::string& info,
             const std::string name):
    Type(type),
    JobInfo(info),
    NeedsSpecificMesher(name.size() > 0),
    MesherName(name)
    {
    }

  //specify a specific mesher is required to run this job
  void specificMesherRequired() { NeedsSpecificMesher = true; }

  //state that any mesh worker can do this job
  void specificMesherOptional() { NeedsSpecificMesher = false; }

  remus::MESH_TYPE type() const { return Type; }

  const std::string& jobInfo() const { return JobInfo; }

  bool requiresSpecificMesher() const { return NeedsSpecificMesher; }
  const std::string& specificMesher() const { return MesherName; }

private:
  remus::MESH_TYPE Type;
  std::string JobInfo;
  bool NeedsSpecificMesher;
  std::string MesherName;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::JobRequest& request)
{
  //convert a request to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << request.type() << std::endl;
  buffer << request.jobInfo() << std::endl;
  buffer << request.specificMesher() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::JobRequest to_JobRequest(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(msg);

  int t;
  std::string data,name;
  buffer >> t;
  buffer >> data;
  buffer >> name;

  const remus::MESH_TYPE type = static_cast<remus::MESH_TYPE>(t);
  return remus::JobRequest(type,data,name);
}

//------------------------------------------------------------------------------
inline remus::JobRequest to_JobRequest(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobRequest( std::string(data,size) );
}

}
#endif
