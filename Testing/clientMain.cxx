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

    std::vector<meshserver::common::JobStatus> js;
    //fill the status array.
    for(int i=0; i < jobs.size(); ++i)
      {
      js.push_back(c.jobStatus(meshserver::MESH2D,jobs.at(i)));
      }

    while(jobs.size() > 0)
      {
      for(int i=0; i < jobs.size(); ++i)
        {
        meshserver::common::JobStatus newStatus =
            c.jobStatus(meshserver::MESH2D,jobs.at(i));
        meshserver::common::JobStatus oldStatus =
            js.at(i);
        js[i]=newStatus;

        if(newStatus.Status != oldStatus.Status ||
           newStatus.Progress != oldStatus.Progress)
          {
          std::cout << "job id " << jobs.at(i) << std::endl;
          if( newStatus.Status == meshserver::IN_PROGRESS)
            {
            std::cout << "progress is " << newStatus.Progress << std::endl;
            }
          else
            {
            std::cout << " status of job is: " << meshserver::to_string(newStatus.Status)  << std::endl;
            }
          }
        if(newStatus.Status == meshserver::FINISHED)
          {
          jobs.erase(jobs.begin()+i);
          js.erase(js.begin()+i);
          }
        }
      }
    }
  return 1;
}


