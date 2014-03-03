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

#ifndef remus_proto_JobResult_h
#define remus_proto_JobResult_h

#include <algorithm>
#include <string>
#include <sstream>

#include <remus/proto/conversionHelpers.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

//Job result holds the result that the worker generated for a given job.
//The Data string will hold the actual job result, be it a file path or a custom
//serialized data structure.

namespace remus {
namespace proto {
class JobResult
{
public:
  //construct am invalid JobResult
  JobResult(const boost::uuids::uuid& id):
    JobId(id),
    Data()
    {}

  //construct a JobResult. The result should be considered invalid if the
  //data length is zero
  JobResult(const boost::uuids::uuid& id, const std::string& d):
    JobId(id),
    Data(d)
    {}

  bool valid() const { return Data.size() > 0; }

  const boost::uuids::uuid& id() const { return JobId; }
  const std::string& data() const { return Data; }

private:
  boost::uuids::uuid JobId;
  std::string Data; //data of the result of a job
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobResult& status)
{
  //convert a job detail to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::ostringstream buffer;
  buffer << status.id() << std::endl;
  buffer << status.data().length() << std::endl;
  remus::internal::writeString(buffer, status.data());
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::proto::JobResult to_JobResult(const std::string& status)
{
  //convert a job detail from a string, used as a hack to serialize

  std::istringstream buffer(status);

  boost::uuids::uuid id;
  int dataLen;
  std::string data;

  buffer >> id;
  buffer >> dataLen;
  data = remus::internal::extractString(buffer,dataLen);

  return remus::proto::JobResult(id,data);
}


//------------------------------------------------------------------------------
inline remus::proto::JobResult to_JobResult(const char* data, std::size_t size)
{
  std::string temp(data,size);
  return to_JobResult( temp );
}


}
}

#endif
