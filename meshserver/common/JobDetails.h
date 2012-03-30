/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_JobDetails_h
#define __meshserver_common_JobDetails_h

#include <string>
#include <sstream>

namespace meshserver {
namespace common {

struct JobDetails
{
  std::string JobId;
  std::string Path; //path to the job file to start

  JobDetails(const std::string& id, const std::string& p):
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

  std::string id;
  std::string path;
  buffer >> id;
  buffer >> path;
  return meshserver::common::JobDetails(id,path);
}

}
#endif
