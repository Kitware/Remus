/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_JobResult_h
#define __meshserver_common_JobResult_h

#include <string>
#include <sstream>

namespace meshserver {
namespace common {

struct JobResult
{
  std::string JobId;
  std::string Path; //path to the job file to start

  JobResult(const std::string& id, const std::string& p):
    JobId(id),
    Path(p)
    {}
};
}

//------------------------------------------------------------------------------
inline std::string to_string(const meshserver::common::JobResult& status)
{
  //convert a job detail to a string, used as a hack to serialize
  //encoding is simple, contents newline seperated
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Path << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline meshserver::common::JobResult to_JobResult(const std::string& status)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(status);

  std::string id;
  std::string path;
  buffer >> id;
  buffer >> path;
  return meshserver::common::JobResult(id,path);
}

//------------------------------------------------------------------------------
inline meshserver::common::JobResult to_JobResult(const char* data, int size)
{
  //convert a job status from a string, used as a hack to serialize
  return to_JobResult( std::string(data,size) );
}

}
#endif
