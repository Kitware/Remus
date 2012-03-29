/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_JobDetails_h
#define __meshserver_common_JobDetails_h

#include <string>

namespace meshserver {
namespace common {

struct JobDetails
{
  const std::string Id;
  const std::string Path; //path to the job file to start

  JobDetails(const std::string& id, const std::string& p):
    Id(id),
    Path(p)
    {}
};
}
}
#endif
