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

#include <remus/proto/JobStatus.h>

#include <remus/common/ConversionHelper.h>

#include <sstream>

#include <iostream>

namespace remus {
namespace proto {

//------------------------------------------------------------------------------
JobStatus::JobStatus(const boost::uuids::uuid& jid, remus::STATUS_TYPE statusType):
  JobId(jid),
  Status(statusType),
  Progress(statusType)
{
}

//will make sure that the progress value is between 1 and 100 inclusive
//on both ends. Progress value of zero is used when the status type is not progress
//------------------------------------------------------------------------------
JobStatus::JobStatus(const boost::uuids::uuid& jid,
                     const remus::proto::JobProgress& jprogress):
  JobId(jid),
  Status(remus::IN_PROGRESS),
  Progress(jprogress)
{
}

//------------------------------------------------------------------------------
void JobStatus::mergeStatus( const remus::proto::JobStatus& status )
{
  this->Status = status.Status;
  this->Progress.setValue(status.Progress.value());
  this->Progress.appendMessage(status.Progress.message());
}

//------------------------------------------------------------------------------
void JobStatus::updateProgress( const remus::proto::JobProgress& prog )
{
  this->Status = remus::IN_PROGRESS;
  this->Progress = prog;
}

//------------------------------------------------------------------------------
void JobStatus::clearProgress()
{
  this->Progress.setMessage("");
}

//------------------------------------------------------------------------------
void JobStatus::serialize(std::ostream& buffer) const
{ //note don't use std::endl as it flushes stream and decrease performance
  buffer << this->id() << '\n';
  buffer << this->status() << '\n';
  buffer << this->progress() << '\n';
}

//------------------------------------------------------------------------------
JobStatus::JobStatus(std::istream& buffer)
{
  int t;
  buffer >> this->JobId;
  buffer >> t;
  buffer >> this->Progress;
  this->Status = static_cast<remus::STATUS_TYPE>(t);
}

//------------------------------------------------------------------------------
std::string to_string(const remus::proto::JobStatus& status)
{
  //convert a job status to a string. We will always send the progress
  std::ostringstream buffer;
  buffer << status;
  return buffer.str();
}

//------------------------------------------------------------------------------
remus::proto::JobStatus to_JobStatus(const std::string& msg)
{
  std::istringstream buffer(msg);
  return remus::proto::JobStatus(buffer);
}


}
}
