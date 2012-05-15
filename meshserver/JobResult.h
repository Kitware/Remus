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

#ifndef __meshserver_JobResult_h
#define __meshserver_JobResult_h

#include <string>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace meshserver {
struct JobResult
{
  boost::uuids::uuid JobId;
  std::string Path; //path to the job file to start

  explicit JobResult(const boost::uuids::uuid& id):
    JobId(id),
    Path()
    {}

  JobResult(const boost::uuids::uuid& id, const std::string& p):
    JobId(id),
    Path(p)
    {}
};

//------------------------------------------------------------------------------
inline std::string to_string(const meshserver::JobResult& status)
{
  //convert a job detail to a string, used as a hack to serialize
  //encoding is simple, contents newline seperated
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Path << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline meshserver::JobResult to_JobResult(const std::string& status)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(status);

  boost::uuids::uuid id;
  std::string path;
  buffer >> id;
  buffer >> path;
  return meshserver::JobResult(id,path);
}

//------------------------------------------------------------------------------
inline meshserver::JobResult to_JobResult(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobResult( std::string(data,size) );
}

}
#endif
