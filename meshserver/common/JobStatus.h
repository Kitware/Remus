/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_JobStatus_h
#define __meshserver_common_JobStatus_h

#include <string>
#include <sstream>
#include <algorithm>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <meshserver/common/meshServerGlobals.h>

namespace
{
inline int valid_progress_value(int v)
{
  v = std::min(v,100);
  v = std::max(v,1);
  return v;
}
}

namespace meshserver {
namespace common {
struct JobStatus
{
  boost::uuids::uuid JobId;
  meshserver::STATUS_TYPE Status;
  int Progress;

  JobStatus(const boost::uuids::uuid& id, meshserver::STATUS_TYPE stat):
    JobId(id),
    Status(stat),
    Progress(0)
    {}

  //will make sure that the progress value is between 1 and 100 inclusive
  //on both ends. Progress value of zero is used when the status type is not progress
  JobStatus(const boost::uuids::uuid& id, meshserver::STATUS_TYPE stat, int progress):
    JobId(id),
    Status(stat),
    Progress( valid_progress_value(progress) )
    {}
};
}

//------------------------------------------------------------------------------
inline std::string to_string(const meshserver::common::JobStatus& status)
{
  //convert a job status to a string, used as a hack to serialize
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Status << std::endl;
  buffer << status.Progress << std::endl;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline meshserver::common::JobStatus to_JobStatus(const std::string& status)
{
  //convert a job status from a string, used as a hack to serialize
  std::stringstream buffer(status);

  boost::uuids::uuid id;
  int t;
  int p;

  buffer >> id;
  buffer >> t;
  buffer >> p;

  const meshserver::STATUS_TYPE type = static_cast<meshserver::STATUS_TYPE>(t);

  return meshserver::common::JobStatus(id,type,p);
}

//------------------------------------------------------------------------------
inline meshserver::common::JobStatus to_JobStatus(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobStatus( std::string(data,size) );
}


}
#endif
