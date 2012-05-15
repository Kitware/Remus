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

#ifndef __meshserver_common_JobDetails_h
#define __meshserver_common_JobDetails_h

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>
#include <sstream>

namespace meshserver {
namespace common {

struct JobDetails
{
  boost::uuids::uuid JobId;
  std::string Path; //path to the job file to start

  JobDetails(const boost::uuids::uuid& id, const std::string& p):
    JobId(id),
    Path(p)
    {}
};
}

//------------------------------------------------------------------------------
inline std::string to_string(const meshserver::common::JobDetails& status)
{
  //convert a job detail to a string, used as a hack to serialize
  //encoding is simple, contents newline seperated
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Path << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline meshserver::common::JobDetails to_JobDetails(const std::string& status)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(status);

  boost::uuids::uuid id;
  std::string path;
  buffer >> id;
  buffer >> path;
  return meshserver::common::JobDetails(id,path);
}

//------------------------------------------------------------------------------
inline meshserver::common::JobDetails to_JobDetails(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobDetails( std::string(data,size) );
}

}
#endif
