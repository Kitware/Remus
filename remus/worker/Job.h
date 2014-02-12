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

#include <remus/common/MeshIOType.h>
#include <remus/proto/JobSubmission.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

//The remus::worker::Job class.
// For the server the Job object represents all the information required to
// start an actual mesh job. Like the client side job object the Id and Type
// are  filled, but now we also have a JobSubmission object which contain the
// client side submission information. The worker needs the Id and Type
// information so that it can properly report back status and
// results to the server
namespace remus{
namespace worker{
class Job
{
public:
  enum JobValidity{ INVALID = 0, VALID_JOB, TERMINATE_WORKER};

  //construct an invalid job object
  Job():
    Id(),
    Type(),
    Validity(INVALID),
    Submission()
  {
  }

  //construct a valid server side job object with Id, Type, and Submission
  Job(const boost::uuids::uuid& id,
      const remus::common::MeshIOType& type,
      const remus::proto::JobSubmission& sub):
  Id(id),
  Type(type),
  Validity(VALID_JOB),
  Submission(sub)
  {

  }

  //get if the current job is a valid job
  bool valid() const { return Validity != INVALID && Type.valid(); }

  //Get the status of the message
  JobValidity validityReason() const { return Validity; }
  void updateValidityReason(JobValidity v){ Validity = v; }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const { return Type; }

  //get the submission object that was sent on the client
  const remus::proto::JobSubmission& submission() const
    { return Submission; }

private:
  boost::uuids::uuid Id;
  remus::common::MeshIOType Type;
  JobValidity Validity;

  remus::proto::JobSubmission Submission;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::worker::Job& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << job.type() << std::endl;
  buffer << job.id() << std::endl;
  buffer << job.submission() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::worker::Job to_Job(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::stringstream buffer(msg);

  boost::uuids::uuid id;
  remus::common::MeshIOType type;
  remus::proto::JobSubmission submission;

  buffer >> type;
  buffer >> id;
  buffer >> submission;
  return remus::worker::Job(id,type,submission);
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
