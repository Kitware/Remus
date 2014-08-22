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
#include <remus/proto/JobProgress.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

namespace remus {
namespace proto {

class REMUSPROTO_EXPORT JobStatus
{
public:
  //Construct a job status that has no form of progress value or message
  JobStatus(const boost::uuids::uuid& jid, remus::STATUS_TYPE statusType);

  //will make sure that the progress value is between 1 and 100 inclusive
  //on both ends. Progress value of zero is used when the status type is not progress
  JobStatus(const boost::uuids::uuid& jid, const remus::proto::JobProgress& jprogress);

  //returns the current progress object
  const remus::proto::JobProgress& progress() const
    { return this->Progress; }

  //update the progress values of the status object
  void updateProgress( const remus::proto::JobProgress& prog );

  //returns true if the job is still running on the worker
  bool inProgress() const
    { return this->Status == remus::IN_PROGRESS; }

  //returns true if the job is waiting for a worker
  bool queued() const
    { return this->Status == remus::QUEUED; }

  //returns true if the job is in progress or queued
  bool good() const
    { return queued() || inProgress(); }

  //returns true if the job failed to launch in any way.
  bool failed() const
  {
    return (this->Status == remus::INVALID_STATUS) ||
           (this->Status == remus::FAILED) ||
           (this->Status == remus::EXPIRED);
  }

  //marks the job as being failing to finish
  void markAsFailed()
    { this->Status = remus::FAILED; }

  //returns true if the job is finished and you can get the results from the server.
  bool finished() const
    { return this->Status == remus::FINISHED; }

  //marks the job as being successfully finished
  void markAsFinished()
    { this->Status = remus::FINISHED; }

  //returns true if the job is valid
  bool valid() const
    { return this->Status != remus::INVALID_STATUS; }

  //returns false if the job is valid
  bool invalid() const
    { return this->Status == remus::INVALID_STATUS; }

  //returns the uuid for the job that this status is for
  const boost::uuids::uuid& id() const { return JobId; }

  //get back the status flag type for this job
  remus::STATUS_TYPE status() const { return Status; }

  //overload on the job status object to make it easier to detect when
  //job status has been changed.
  bool operator ==(const JobStatus& b) const
  {
    return (this->JobId == b.JobId)   &&
           (this->Status == b.Status) &&
           (this->Progress == b.Progress);
  }

  //overload on the job status object to make it easier to detect when
  //job status has been changed.
  bool operator !=(const JobStatus& b) const
    { return !(this->operator ==(b)); }

  friend std::ostream& operator<<(std::ostream &os,
                                    const JobStatus &status)
    { status.serialize(os); return os; }
  friend std::istream& operator>>(std::istream &is,
                                  JobStatus &status)
    { status = JobStatus(is); return is; }

private:
  friend remus::proto::JobStatus to_JobStatus(const std::string& msg);

  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobStatus(std::istream& buffer);

  boost::uuids::uuid JobId;
  remus::STATUS_TYPE Status;
  remus::proto::JobProgress Progress;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobStatus& status)
{
  //convert a job status to a string. We will always send the progress
  std::ostringstream buffer;
  buffer << status;
  return buffer.str();
}

//----------------------------------------------------------------------------
inline remus::proto::JobStatus make_JobStatus(const boost::uuids::uuid jid,
                                              int value)
{
  const remus::proto::JobProgress progress(value);
  return remus::proto::JobStatus(jid,progress);
}

//----------------------------------------------------------------------------
inline remus::proto::JobStatus make_JobStatus(const boost::uuids::uuid jid,
                                              const std::string& message)
{
  const remus::proto::JobProgress progress(message);
  return remus::proto::JobStatus(jid,progress);
}

//----------------------------------------------------------------------------
inline remus::proto::JobStatus make_FailedJobStatus(
                                          const boost::uuids::uuid jid,
                                          const std::string& failure_message)
{
  const remus::proto::JobProgress failure_progress(failure_message);
  remus::proto::JobStatus jstatus(jid,failure_progress);
  //create a status with a message marks us as IN_PROGRESS, so we need to move
  //to a FAILED state.
  jstatus.markAsFailed();
  return jstatus;
}

//------------------------------------------------------------------------------
inline remus::proto::JobStatus to_JobStatus(const std::string& msg)
{
  std::istringstream buffer(msg);
  return remus::proto::JobStatus(buffer);
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
