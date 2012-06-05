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

  if(c.canMesh(remus::MESH3D))
    {
    std::string omicronModel="smooth_surface_model.dat";
    std::string jid = c.submitMeshJob(remus::MESH3D,
                                      omicronModel);
    std::cout << "job submitted" << std::endl;

    remus::JobStatus oldStatus = c.jobStatus(remus::MESH3D,jid);
    std::cout << " status of job is: " << remus::to_string(oldStatus.Status)  << std::endl;

    while(oldStatus.Status != remus::FINISHED &&
          oldStatus.Status != remus::FAILED)
      {
      remus::JobStatus newStatus = c.jobStatus(remus::MESH3D,jid);
      if(newStatus.Status != oldStatus.Status ||
         newStatus.Progress != oldStatus.Progress)
        {
        std::cout << "job id " << newStatus.JobId << std::endl;
        if( newStatus.Status == remus::IN_PROGRESS)
          {
          std::cout << "progress is " << newStatus.Progress << std::endl;
          }
        else
          {
          std::cout << " status of job is: " << remus::to_string(newStatus.Status)  << std::endl;
          }
        oldStatus = newStatus;
        }
      }
    }
  return 1;
}


