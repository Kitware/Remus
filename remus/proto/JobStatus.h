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

#ifndef remus_proto_JobStatus_h
#define remus_proto_JobStatus_h

#include <algorithm>
#include <string>
#include <sstream>

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

#include <remus/common/remusGlobals.h>
#include <remus/proto/conversionHelpers.h>
#include <remus/proto/JobProgress.h>

namespace remus {
namespace proto {

class JobStatus
{
public:
  //Construct a job status that has no form of progress value or message
  JobStatus(const boost::uuids::uuid& jid, remus::STATUS_TYPE statusType):
    JobId(jid),
    Status(statusType),
    Progress(statusType)
    {
    }

  //will make sure that the progress value is between 1 and 100 inclusive
  //on both ends. Progress value of zero is used when the status type is not progress
  JobStatus(const boost::uuids::uuid& jid, const remus::proto::JobProgress& jprogress):
    JobId(jid),
    Status(remus::IN_PROGRESS),
    Progress(jprogress)
    {
    }

  //returns the current progress object
  const remus::proto::JobProgress& progress() const
  {
    return this->Progress;
  }

  //update the progress values of the status object
  void updateProgress( const remus::proto::JobProgress& prog )
    {
    this->Status = remus::IN_PROGRESS;
    this->Progress = prog;
    }

  //returns true if the job is still running on the worker
  bool inProgress() const
  {
    return this->Status == remus::IN_PROGRESS;
  }

  //returns true if the job is waiting for a worker
  bool queued() const
  {
    return this->Status == remus::QUEUED;
  }

  //returns true if the job is in progress or queued
  bool good() const
  {
    return queued() || inProgress();
  }

  //returns true if the job failed to launch in any way.
  bool failed() const
  {
    return (this->Status == remus::INVALID_STATUS) ||
           (this->Status == remus::FAILED) ||
           (this->Status == remus::EXPIRED);
  }

  //marks the job as being failing to finish
  void markAsFailed()
  {
    this->Status = remus::FAILED;
  }

  //returns true if the job is finished and you can get the results from the server.
  bool finished() const
  {
    return this->Status == remus::FINISHED;
  }

  //marks the job as being successfully finished
  void markAsFinished()
  {
    this->Status = remus::FINISHED;
  }

  //returns the uuid for the job that this status is for
  const boost::uuids::uuid& id() const { return JobId; }

  //get back the status flag type for this job
  remus::STATUS_TYPE status() const { return Status; }

private:
  boost::uuids::uuid JobId;
  remus::STATUS_TYPE Status;
  remus::proto::JobProgress Progress;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobStatus& status)
{
  //convert a job status to a string, used as a hack to serialize
  std::ostringstream buffer;
  buffer << status.id() << std::endl;
  buffer << status.status() << std::endl;

  //only send progress info, if we are actually a status message that
  //cares about that information
  if(status.status() == remus::IN_PROGRESS)
    {
    buffer << status.progress().value() << std::endl;
    buffer << status.progress().message().size() << std::endl;
    remus::internal::writeString(buffer,status.progress().message());
    }
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::proto::JobStatus to_JobStatus(const std::string& status)
{
  //convert a job status from a string, used as a hack to serialize
  std::istringstream buffer(status);

  boost::uuids::uuid id;
  int t;
  buffer >> id;
  buffer >> t;

  const remus::STATUS_TYPE type = static_cast<remus::STATUS_TYPE>(t);

  remus::proto::JobStatus jstatus(id,type);
  if(type == remus::IN_PROGRESS)
    {
    remus::proto::JobProgress pr(type);
    //if we are progress status message we have two more pieces of info to decode
    int progressValue;
    buffer >> progressValue;
    if(progressValue > 0)
      {
      pr.setValue(progressValue);
      }

    int progressMessageLen;
    std::string progressMessage;

    //this is really important, the progress message can have multiple words and/or
    //new line characters. so we want all of the left over characters in the
    //buffer to be the progress message.
    buffer >>progressMessageLen;
    progressMessage = remus::internal::extractString(buffer,progressMessageLen);

    pr.setMessage(progressMessage);
    jstatus = remus::proto::JobStatus(id,pr);
    }
  return jstatus;
}


//------------------------------------------------------------------------------
inline remus::proto::JobStatus to_JobStatus(const char* data, std::size_t size)
{
  //the data might contain null terminators which on windows
  //makes the data,size construct fail, so instead we use std::copy
  std::string temp(data,size);
  return to_JobStatus( temp );
}


}
}

#endif
