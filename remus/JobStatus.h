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

#ifndef __remus_JobStatus_h
#define __remus_JobStatus_h

#include <algorithm>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <remus/common/remusGlobals.h>

namespace remus {

class JobStatus;

//Job progress is a helper class to easily state what the progress of a currently
//running job is. Progress can be numeric, textual or both.
struct JobProgress
{
  JobProgress():
    Value(-1),
    Message()
    {}

  explicit JobProgress(remus::STATUS_TYPE status):
    Value(-1),
    Message()
    {
    if(status==remus::IN_PROGRESS)
      {
      //if this is a valid in_progress job progress struct, than make
      //value 0
      this->Value = 0;
      }
    }

  explicit JobProgress(int value):
    Value(valid_progress_value(value)),
    Message()
    {}

  explicit JobProgress(const std::string& message):
    Value(0),
    Message(message)
    {}

  JobProgress(int value, const std::string& message):
    Value(valid_progress_value(value)),
    Message(message)
    {}

  //overload on the progress object to make it easier to detect when
  //progress has been changed.
  bool operator ==(const JobProgress& b) const
  {
    return this->Value == b.Value && this->Message == b.Message;
  }

  //overload on the progress object to make it easier to detect when
  //progress has been changed.
  bool operator !=(const JobProgress& b) const
  {
    return !(this->operator ==(b));
  }

  int value() const { return Value; }
  const std::string message() const { return Message; }

  void setValue(int value) { Value = JobProgress::valid_progress_value(value); }
  void setMessage(const std::string& msg) { Message = msg; }


  //make sure that we can't set progress to be outside the valid range.
  static inline int valid_progress_value(int v)
  {
    v = std::min<int>(v,100);
    v = std::max<int>(v,1);
    return v;
  }

  //make it easy to print out the progress, by overloading the << operator
  //when using cout, cerr, etc
  friend std::ostream& operator<< (std::ostream& out, const JobProgress& j)
  {
    if(j.Value < 0)
      {
      out << "Progress: Invalid";
      }
    else if(j.Value > 0 && j.Message.size() > 0)
      {
      out << "Progress[" << j.Value << "%]: " << j.Message;
      }
    else if(j.Value > 0)
      {
      out << "Progress: " << j.Value << "%";
      }
    else
      {
      out << "Progress: " << j.Message;
      }
    return out;
  }

private:
  friend JobStatus to_JobStatus(const std::string& status);
  void setUncheckedValue(int value){Value=value;}

  int Value;
  std::string Message;
};

struct JobStatus
{
  boost::uuids::uuid JobId;
  remus::STATUS_TYPE Status;
  remus::JobProgress Progress;

  //Construct an invalid job status message
  JobStatus():
    JobId(),
    Status(remus::INVALID_STATUS),
    Progress(remus::INVALID_STATUS)
    {}

  //Construct a job status that has no form of progress value or message
  JobStatus(const boost::uuids::uuid& id, remus::STATUS_TYPE stat):
    JobId(id),
    Status(stat),
    Progress(stat)
    {
    }

  //will make sure that the progress value is between 1 and 100 inclusive
  //on both ends. Progress value of zero is used when the status type is not progress
  JobStatus(const boost::uuids::uuid& id, const remus::JobProgress& progress):
    JobId(id),
    Status(remus::IN_PROGRESS),
    Progress(progress)
    {

    }

  //returns true if the job is waiting for a worker
  bool queued() const
  {
    return this->Status == remus::QUEUED;
  }

  //returns true if the job is still running on the worker
  bool inProgress() const
  {
    return this->Status == remus::IN_PROGRESS;
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

  //returns true if the job is finished and you can get the results from the server.
  bool finished() const
  {
    return this->Status == remus::FINISHED;
  }

};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::JobStatus& status)
{
  //convert a job status to a string, used as a hack to serialize
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Status << std::endl;

  //only send progress info, if we are actually a status message that
  //cares about that information
  if(status.Status == remus::IN_PROGRESS)
    {
    buffer << status.Progress.value() << std::endl;
    buffer << status.Progress.message().size() << status.Progress.message() << std::endl;
    }
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::JobStatus to_JobStatus(const std::string& status)
{
  //convert a job status from a string, used as a hack to serialize
  std::stringstream buffer(status);

  boost::uuids::uuid id;
  int t;
  buffer >> id;
  buffer >> t;

  const remus::STATUS_TYPE type = static_cast<remus::STATUS_TYPE>(t);
  if(type!=remus::IN_PROGRESS)
    {
    return remus::JobStatus(id,type);
    }
  else
    {
    //if we are progress status message we have two more pieces of info to decode
    int progressValue;
    buffer >> progressValue;

    int progressMessageLen;
    std::string progressMessage;

    //this is really important, the progress message can have multiple words and/or
    //new line characters. so we want all of the left over characters in the
    //buffer to be the progress message.
    buffer >>progressMessageLen;
    progressMessage = remus::internal::extractString(buffer,progressMessageLen);

    //don't use any of the constructors as we want a -1 or a 0 value
    //not to be converted to a value of 1.
    JobProgress pr(progressMessage);
    pr.setUncheckedValue(progressValue);

    return remus::JobStatus(id,pr);
    }
}

//------------------------------------------------------------------------------
inline remus::JobStatus to_JobStatus(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobStatus( std::string(data,size) );
}

}
#endif
