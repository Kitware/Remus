/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_JobStatus_h
#define __meshserver_common_JobStatus_h

#include <string>
#include <algorithm>

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
  const std::string JobId;
  const meshserver::STATUS_TYPE Status;
  const int Progress;

  JobStatus(const std::string& id, meshserver::STATUS_TYPE stat):
    JobId(id),
    Status(stat),
    Progress(0)
    {}

  //will make sure that the progress value is between 1 and 100 inclusive
  //on both ends. Progress value of zero is used when the status type is not progress
  JobStatus(const std::string& id, meshserver::STATUS_TYPE stat, int progress):
    JobId(id),
    Status(stat),
    Progress( valid_progress_value(progress) )
    {}



};
}
}
#endif
