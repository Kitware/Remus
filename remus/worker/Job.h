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
//suppress warnings inside boost headers for gcc and clang
#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <boost/uuid/uuid_io.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

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
    Submission(),
    Validity(INVALID)
  {
  }

  //construct a valid server side job object with Id, Type, and Submission
  Job(const boost::uuids::uuid& jid,
      const remus::proto::JobSubmission& sub):
  Id(jid),
  Submission(sub),
  Validity(VALID_JOB)
  {

  }

  //get if the current job is a valid job
  bool valid() const { return Validity != INVALID && this->type().valid(); }

  //Get the status of the message
  JobValidity validityReason() const { return Validity; }
  void updateValidityReason(JobValidity v){ Validity = v; }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const
    { return this->Submission.type(); }

  //get the submission object that was sent on the client
  const remus::proto::JobSubmission& submission() const
    { return Submission; }

  //helper method to easily get the contents of a given key
  //inside the submission as a std::string.
  std::string details(const std::string key) const
   {
   typedef remus::proto::JobSubmission::const_iterator it;
   it i = this->Submission.find(key);
   if( i != this->Submission.end() )
    {
    return std::string(i->second.data(),i->second.dataSize());
    }
   return std::string();
   }

private:
  boost::uuids::uuid Id;
  remus::proto::JobSubmission Submission;

  JobValidity Validity;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::worker::Job& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::ostringstream buffer;
  buffer << job.id() << std::endl;
  buffer << job.submission() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::worker::Job to_Job(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::istringstream buffer(msg);

  boost::uuids::uuid id;
  remus::proto::JobSubmission submission;

  buffer >> id;
  buffer >> submission;
  return remus::worker::Job(id,submission);
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
