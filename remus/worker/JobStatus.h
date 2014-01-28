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

#ifndef remus_worker_JobStatus_h
#define remus_worker_JobStatus_h

#include <algorithm>
#include <string>
#include <sstream>
#include <cassert>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <remus/common/JobProgress.h>
#include <remus/common/remusGlobals.h>


namespace remus {
namespace worker {

//bring JobProgress into the worker namespace
using remus::common::JobProgress;

class JobStatus
{
public:
  remus::worker::JobProgress Progress;
  boost::uuids::uuid JobId;
  remus::STATUS_TYPE Status;


  //Construct a job status that has no form of progress value or message
  JobStatus(const boost::uuids::uuid& id, remus::STATUS_TYPE stat):
    JobId(id),
    Status(stat),
    Progress(stat)
    {
    }

  //will make sure that the progress value is between 1 and 100 inclusive
  //on both ends. Progress value of zero is used when the status type is not progress
  JobStatus(const boost::uuids::uuid& id,
            const remus::worker::JobProgress& progress):
    JobId(id),
    Status(remus::IN_PROGRESS),
    Progress(progress)
    {

    }

  //get a read only reference to current progress
  const remus::worker::JobProgress& progress() const
  {
    return this->Progress;
  }

  //update the progress values of the status object
  void updateProgress( const remus::worker::JobProgress& progress )
    {
    this->Status = remus::IN_PROGRESS;
    this->Progress = progress;
    }

  //returns true if our status is currently set to IN_PROGRESS
  bool inProgress() const
  {
    return this->Status == remus::IN_PROGRESS;
  }

  //returns true if the job is finished.
  bool finished() const
  {
    return this->Status == remus::FINISHED;
  }

  //returns true if the job failed to launch in any way.
  bool failed() const
  {
    return (this->Status == remus::INVALID_STATUS) ||
           (this->Status == remus::FAILED) ||
           (this->Status == remus::EXPIRED);
  }

  //marks the job as being failing to finish, and will be reported
  //as a failure when sent back to the server
  void hasFailed()
  {
    this->Status = remus::FAILED;
  }

  //marks the job as being successfully finished
  void hasFinished()
  {
    this->Status = remus::FINISHED;
  }


};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::worker::JobStatus& status)
{
  //convert a job status to a string, used as a hack to serialize
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Status << std::endl;

  //only send progress info, if we are actually a status message that
  //cares about that information
  if(status.Status == remus::IN_PROGRESS)
    {
    buffer << status.progress().value() << std::endl;
    buffer << status.progress().message().size() << std::endl;
    remus::internal::writeString(buffer,status.progress().message());
    }
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::worker::JobStatus to_JobStatus(const std::string& status)
{
  //convert a job status from a string, used as a hack to serialize
  std::stringstream buffer(status);

  boost::uuids::uuid id;
  int t;
  buffer >> id;
  buffer >> t;

  const remus::STATUS_TYPE type = static_cast<remus::STATUS_TYPE>(t);
  remus::worker::JobStatus jstatus(id,type);
  if(type==remus::IN_PROGRESS)
    {
    remus::worker::JobProgress pr(type);
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
    jstatus = remus::worker::JobStatus(id,pr);
    }
  return jstatus;
}


//------------------------------------------------------------------------------
inline remus::worker::JobStatus to_JobStatus(const char* data, int size)
{
  //the data might contain null terminators which on windows
  //makes the data,size construct fail, so instead we memcpy
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobStatus( temp );
}

}
}

#endif
