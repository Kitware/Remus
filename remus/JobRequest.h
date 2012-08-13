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

#include <remus/common/remusGlobals.h>

//A job request has two purposes. First it is sent to the server to determine
//if the mesh type and data model that the job request has is supported.
//Secondly it is used to transport the actual job to the server

//Note: Currently no server supports the ability to restrict a mesh request
//to a single named mesher. This option is expected to be supported in the future.
//For now we replicate this feature by making <meshType,inputMeshDataType> be
//unique for each mesher
namespace remus{
class JobRequest
{
public:

  //Construct a job request with only a mesher type given. This is used to
  //when asking a server if it supports a type of mesh
  JobRequest(remus::MESH_OUTPUT_TYPE outputMeshType,
             remus::MESH_INPUT_TYPE inputFileType):
    CombinedType(inputFileType,outputMeshType),
    JobInfo()
    {
    }

  //Construct a job request with a mesh type and the info required by the worker
  //to run the job. This is used when submitting a job from the client to the server.
  JobRequest(remus::MESH_OUTPUT_TYPE outputMeshType,
             remus::MESH_INPUT_TYPE inputFileType,
             const std::string& info):
    CombinedType(inputFileType,outputMeshType),
    JobInfo(info)
    {
    }

  //Construct a job request with the given types held inside the MESH_TYPE object
  explicit JobRequest(remus::MESH_TYPE combinedType)
    {

    }

  //Construct a job request with the given types held inside the MESH_TYPE object
  JobRequest(remus::MESH_TYPE combinedType, const std::string& info)
    {

    }

  //constructs a variable that represents the combination of the input
  //and output type as a single integer
  remus::MESH_TYPE type() const { return this->CombinedType; }

  remus::MESH_OUTPUT_TYPE outputType() const { return CombinedType.outputType(); }
  remus::MESH_INPUT_TYPE inputType() const { return CombinedType.inputType(); }
  const std::string& jobInfo() const { return JobInfo; }

private:
  remus::MESH_TYPE CombinedType;
  std::string JobInfo;

};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::JobRequest& request)
{
  //convert a request to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << request.type() << std::endl;
  buffer << request.jobInfo().length() << request.jobInfo() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::JobRequest to_JobRequest(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::stringstream buffer(msg);


  int out, in, dataLen;
  std::string data;
  buffer >> out; //out type first
  buffer >> in; //in type second

  buffer >> dataLen;
  data = remus::internal::extractString(buffer,dataLen);

  const remus::MESH_OUTPUT_TYPE outType = static_cast<remus::MESH_OUTPUT_TYPE>(out);
  const remus::MESH_INPUT_TYPE inType = static_cast<remus::MESH_INPUT_TYPE>(in);
  return remus::JobRequest(outType,inType,data);
}

//------------------------------------------------------------------------------
inline remus::JobRequest to_JobRequest(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobRequest( std::string(data,size) );
}

}
#endif
