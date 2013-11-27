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

#ifndef remus_worker_JobResult_h
#define remus_worker_JobResult_h

#include <string>
#include <sstream>

#include <remus/common/remusGlobals.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

//Job result holds the result that the worker generated for a given job.
//The Data string will hold the actual job result, be it a file path or a custom
//serialized data structure.

namespace remus {
namespace worker {

struct JobResult
{
  boost::uuids::uuid JobId;
  std::string Data; //data of the result of a job

  //construct a job result with no data. This would be considered to be an
  //invalid job result
  explicit JobResult(const boost::uuids::uuid& id):
    JobId(id),
    Data()
    {}

  //construct a result with actual data.
  JobResult(const boost::uuids::uuid& id, const std::string& d):
    JobId(id),
    Data(d)
    {}

  bool valid() const { return Data.size() > 0; }
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::worker::JobResult& status)
{
  //convert a job detail to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Data.length() << std::endl;
  remus::internal::writeString(buffer, status.Data);
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::worker::JobResult to_JobResult(const std::string& status)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(status);

  boost::uuids::uuid id;
  int dataLen;
  std::string data;

  buffer >> id;
  buffer >> dataLen;
  data = remus::internal::extractString(buffer,dataLen);

  return remus::worker::JobResult(id,data);
}


//------------------------------------------------------------------------------
inline remus::worker::JobResult to_JobResult(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  std::string temp(size,char());
  memcpy(const_cast<char*>(temp.c_str()),data,size);
  return to_JobResult( temp );
}


}
}

#endif
