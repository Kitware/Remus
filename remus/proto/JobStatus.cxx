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

#include <remus/proto/conversionHelpers.h>

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
void JobStatus::updateProgress( const remus::proto::JobProgress& prog )
{
  this->Status = remus::IN_PROGRESS;
  this->Progress = prog;
}

//------------------------------------------------------------------------------
void JobStatus::serialize(std::ostream& buffer) const
{
  buffer << this->id() << std::endl;
  buffer << this->status() << std::endl;
  buffer << this->progress() << std::endl;
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


}
}
