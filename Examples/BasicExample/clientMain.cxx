/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/client/Client.h>

#include <vector>
#include <iostream>
int main ()
{
  remus::client::ServerConnection conn;
  remus::client::Client c(conn);

  if(c.canMesh(remus::MESH2D))
    {
    std::vector<std::string> jobs;
    for(int i=0; i < 12; ++i)
      {
      std::string jid = c.submitMeshJob(remus::MESH2D,"TEST");
      if(jid.size()>0)
        {
        jobs.push_back(jid);
        }
      }

    std::vector<remus::JobStatus> js;
    //fill the status array.
    for(int i=0; i < jobs.size(); ++i)
      {
      js.push_back(c.jobStatus(remus::MESH2D,jobs.at(i)));
      }

    while(jobs.size() > 0)
      {
      for(int i=0; i < jobs.size(); ++i)
        {
        remus::JobStatus newStatus =
            c.jobStatus(remus::MESH2D,jobs.at(i));
        remus::JobStatus oldStatus =
            js.at(i);
        js[i]=newStatus;

        if(newStatus.Status != oldStatus.Status ||
           newStatus.Progress != oldStatus.Progress)
          {
          std::cout << "job id " << jobs.at(i) << std::endl;
          if( newStatus.Status == remus::IN_PROGRESS)
            {
            std::cout << "progress is " << newStatus.Progress << std::endl;
            }
          else
            {
            std::cout << " status of job is: " << remus::to_string(newStatus.Status)  << std::endl;
            }
          }
        if(newStatus.Status == remus::FINISHED ||
           newStatus.Status == remus::FAILED)
          {
          jobs.erase(jobs.begin()+i);
          js.erase(js.begin()+i);
          }
        }
      }
    }
  return 1;
}


