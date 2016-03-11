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

#include <string>

#include <remus/common/MeshIOType.h>
#include <remus/proto/JobSubmission.h>
#include <remus/proto/WorkerJob.h>

//suppress warnings inside boost headers for gcc and clang
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE


namespace remus{
namespace worker{

// The remus::worker::Job class.
// For the server the Job object represents all the information required to
// start an actual mesh job. Like the client side job object the Id and Type
// are  filled, but now we also have a JobSubmission object which contain the
// client side submission information. The worker needs the Id and Type
// information so that it can properly report back status and
// results to the server
typedef remus::proto::WorkerJob Job;


//------------------------------------------------------------------------------
inline std::string to_string(const remus::worker::Job& job)
{
  return remus::proto::to_string(job);
}


//------------------------------------------------------------------------------
inline remus::worker::Job to_Job(const std::string& msg)
{
  return remus::proto::to_WorkerJob(msg);
}

//------------------------------------------------------------------------------
inline remus::worker::Job to_Job(const char* data, int size)
{
  //convert a job from a string, used as a hack to serialize
  const std::string temp(data,size);
  return to_Job( temp );
}

}
}

#endif
