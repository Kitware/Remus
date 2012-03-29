/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/Client.h>

#include <vector>
#include <iostream>
int main ()
{
  meshserver::Client c;

  if(c.canMesh(meshserver::MESH2D))
    {
    std::vector<std::string> jobIds;
    for(int i=0; i < 2; ++i)
      {
      std::string jid = c.submitMeshJob(meshserver::MESH2D,"TEST");
      if(jid.size()>0)
        {
        jobIds.push_back(jid);
        }
      }
    for(int i=0; i < jobIds.size(); ++i)
      {
      std::cout << "job id " << jobIds.at(i) << std::endl;
      meshserver::common::JobStatus status = c.jobStatus(meshserver::MESH2D,jobIds.at(i));
      std::cout << " status of job is: " << meshserver::to_string(status.Status)  << std::endl;
      }
    }
  return 1;
}
