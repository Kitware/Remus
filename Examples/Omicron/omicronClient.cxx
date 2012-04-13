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

  if(c.canMesh(meshserver::MESH3D))
    {
    std::string omicronModel="smooth_surface_model.dat";
    std::string jid = c.submitMeshJob(meshserver::MESH3D,
                                      omicronModel);
    std::cout << "job submitted" << std::endl;

    meshserver::common::JobStatus oldStatus = c.jobStatus(meshserver::MESH3D,jid);
    std::cout << " status of job is: " << meshserver::to_string(oldStatus.Status)  << std::endl;

    while(oldStatus.Status != meshserver::FINISHED &&
          oldStatus.Status != meshserver::FAILED)
      {
      meshserver::common::JobStatus newStatus = c.jobStatus(meshserver::MESH3D,jid);
      if(newStatus.Status != oldStatus.Status ||
         newStatus.Progress != oldStatus.Progress)
        {
        std::cout << "job id " << newStatus.JobId << std::endl;
        if( newStatus.Status == meshserver::IN_PROGRESS)
          {
          std::cout << "progress is " << newStatus.Progress << std::endl;
          }
        else
          {
          std::cout << " status of job is: " << meshserver::to_string(newStatus.Status)  << std::endl;
          }
        oldStatus = newStatus;
        }
      }
    }
  return 1;
}


