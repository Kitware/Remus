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

#include <remus/common/CompilerInformation.h>
#if defined(REMUS_MSVC)
 #pragma warning(push)
 #pragma warning(disable:4996)  /*using non checked iterators*/
#endif

#include <remus/proto/WorkerJob.h>

#include <sstream>

//suppress warnings inside boost headers for gcc and clang
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid_io.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace remus{
namespace proto{

//------------------------------------------------------------------------------
WorkerJob::WorkerJob():
    Id(),
    Submission(),
    Validity(INVALID)
{
}

//------------------------------------------------------------------------------
WorkerJob::WorkerJob(const boost::uuids::uuid& jid,
                     const remus::proto::JobSubmission& sub):
  Id(jid),
  Submission(sub),
  Validity(VALID_JOB)
{

}

//------------------------------------------------------------------------------
std::string WorkerJob::details(const std::string key) const
{
  typedef remus::proto::JobSubmission::const_iterator it;
  it i = this->Submission.find(key);
  if( i != this->Submission.end() )
    {
    return std::string(i->second.data(),i->second.dataSize());
    }
  return std::string();
}

//------------------------------------------------------------------------------
bool WorkerJob::details(const std::string& key, remus::proto::JobContent& value)
{
  typedef remus::proto::JobSubmission::const_iterator it;
  it attIt = this->Submission.find(key);
  if(attIt == this->Submission.end())
    {
    return false;
    }
  value = attIt->second;
  return true;
}

//------------------------------------------------------------------------------
std::string to_string(const remus::proto::WorkerJob& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::ostringstream buffer;
  buffer << job.id() << std::endl;
  buffer << job.submission() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
remus::proto::WorkerJob to_WorkerJob(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::istringstream buffer(msg);

  boost::uuids::uuid id;
  remus::proto::JobSubmission submission;

  buffer >> id;
  buffer >> submission;

  return remus::proto::WorkerJob(id,submission);
}

}
}

#if defined(REMUS_MSVC)
#pragma warning(pop)
#endif
