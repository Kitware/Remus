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

#ifndef remus_worker_Job_h
#define remus_worker_Job_h

#include <algorithm>
#include <string>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <remus/common/MeshIOType.h>

//The remus::worker::Job class.
// For the server the Job object represents all the information required to
// start an actual mesh job. Like the client side job object the Id and Type
// are  filled, but now we also have a JobDetails section will contain the data that
// was submitted with the JobRequest object. The worker needs the Id and Type
// information so that it can properly report back status and results to the server
namespace remus{
namespace worker{
class Job
{
public:
  enum JobValidity{ INVALID = 0, VALID_JOB, TERMINATE_WORKER};

  //construct an invalid job object
  explicit Job(JobValidity js = INVALID):
    Id(),
    Type(),
    JobDetails(),
    Validity(js)
  {
  }

  //construct a valid server side job object with Id, Type, and JobDetails
  Job(const boost::uuids::uuid& id,
      const remus::common::MeshIOType& type,
      const std::string& data,
      JobValidity js = VALID_JOB):
  Id(id),
  Type(type),
  JobDetails(data),
  Validity(js)
  {

  }

  //get if the current job is a valid job
  bool valid() const { return Validity != INVALID || Type.valid(); }

  //Get the status of the message
  JobValidity validityReason() const { return Validity; }
  void updateValidityReason(JobValidity v){ Validity = v; }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const { return Type; }

  //get the details of the job
  const std::string& details() const { return JobDetails; }

private:
  boost::uuids::uuid Id;
  remus::common::MeshIOType Type;
  std::string JobDetails;
  JobValidity Validity;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::worker::Job& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << job.type() << std::endl;
  buffer << job.id() << std::endl;
  buffer << job.details().length() << std::endl;
  remus::internal::writeString(buffer, job.details());
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::worker::Job to_Job(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::stringstream buffer(msg);

  boost::uuids::uuid id;
  remus::common::MeshIOType type;
  int dataLen;
  std::string data;

  buffer >> type;
  buffer >> id;
  buffer >> dataLen;
  data = remus::internal::extractString(buffer,dataLen);
  return remus::worker::Job(id,type,data);
}


//------------------------------------------------------------------------------
inline remus::worker::Job to_Job(const char* data, int size)
{
  //convert a job from a string, used as a hack to serialize
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_Job( temp );
}

}
}

#endif
