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
    std::vector<std::string> jobs;
    for(int i=0; i < 2; ++i)
      {
      std::string jid = c.submitMeshJob(meshserver::MESH2D,"TEST");
      if(jid.size()>0)
        {
        jobs.push_back(jid);
        }
      }
    while(jobs.size() > 0)
      {
      for(int i=0; i < jobs.size(); ++i)
        {
        std::cout << "job id " << jobs.at(i) << std::endl;
        meshserver::common::JobStatus status = c.jobStatus(meshserver::MESH2D,jobs.at(i));

        if( status.Status == meshserver::IN_PROGRESS)
          {
          std::cout << "progress is " << status.Progress << std::endl;
          }
        else
          {
          std::cout << " status of job is: " << meshserver::to_string(status.Status)  << std::endl;
          }

        if(status.Status == meshserver::FINISHED)
          {
          jobs.erase(jobs.begin()+i);
          }
        std::cout << std::endl;
        }
      }
    }
  return 1;
}
