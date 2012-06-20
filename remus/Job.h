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

#ifndef __remus_Job_h
#define __remus_Job_h

#include <string>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <remus/common/remusGlobals.h>

//The remus::Job class has two purposes.
// For the client side interface it holds the Id and Type of a submitted job.
// The client interface is given a job object after submitting a JobRequest.
// For the client the JobDetails section should be empty as you don't need
// to know what data was submitted as part of the JobRequest.

// For the server the Job object represents all the information required to
// start an actual mesh job. Like the client side job object the Id and Type
// are  filled, but also the JobDetails section will contain the data that
// was submitted with the JobRequest object. The worker needs the Id and Type
// information so that it can properly report back status and results to the server
namespace remus{
class Job
{
public:

  //construct an invalid job object
  Job():
    Id(),
    Type(remus::INVALID_MESH),
    JobDetails()
  {
  }

  //construct a valid client side job object with only an Id and Type
  Job(const boost::uuids::uuid& id,
      const remus::MESH_TYPE& type):
  Id(id),
  Type(type),
  JobDetails()
  {
  }

  //construct a valid server side job object with Id, Type, and JobDetails
  Job(const boost::uuids::uuid& id,
      const remus::MESH_TYPE& type,
      const std::string& data):
  Id(id),
  Type(type),
  JobDetails(data)
  {

  }

  //get if the current job is a valid job
  bool valid() const { return remus::INVALID_MESH != Type; }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::MESH_TYPE& type() const { return Type; }

  //get the details of the job, only a call that the worker should expect
  //to return a non empty string
  const std::string& details() const { return JobDetails; }

private:
  boost::uuids::uuid Id;
  remus::MESH_TYPE Type;
  std::string JobDetails;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::Job& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << job.type() << std::endl;
  buffer << job.id() << std::endl;
  buffer << job.details() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::Job to_Job(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(msg);

  boost::uuids::uuid id;
  int t;
  std::string data;
  buffer >> t;
  buffer >> id;
  buffer >> data;

  const remus::MESH_TYPE type = static_cast<remus::MESH_TYPE>(t);
  return remus::Job(id,type,data);
}

//------------------------------------------------------------------------------
inline remus::Job to_Job(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_Job( std::string(data,size) );
}
}
#endif
