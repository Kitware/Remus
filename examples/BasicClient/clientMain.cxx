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
  //create a default server connection
  remus::client::ServerConnection conn;

  //create a client object that will connect to the default remus server
  remus::Client c(conn);

  //we create a basic job request for a mesh2d job, with the data contents of "TEST"
  remus::JobRequest request(remus::RAW_EDGES,remus::MESH2D, "TEST THE APPLICATION BY PASSING SPACES");
  if(c.canMesh(request))
    {
    //if we can mesh 2D mesh jobs, we are going to submit 8 jobs to the server
    //for meshing
    std::vector<remus::Job> jobs;
    for(int i=0; i < 8; ++i)
      {
      remus::Job job = c.submitJob(request);
      jobs.push_back(job);
      }

    //get the initial status of all the jobs that we have
    std::vector<remus::JobStatus> js;
    for(int i=0; i < jobs.size(); ++i)
      {
      js.push_back(c.jobStatus(jobs.at(i)));
      }

    //While we have jobs still running on the server report back
    //each time the status of the job changes
    while(jobs.size() > 0)
      {
      for(int i=0; i < jobs.size(); ++i)
        {
        //update the status with the latest status and compare it too
        //the previously held status
        remus::JobStatus newStatus = c.jobStatus(jobs.at(i));
        remus::JobStatus oldStatus = js.at(i);
        js[i]=newStatus;

        //if the status or progress value has changed report it to the cout stream
        if(newStatus.Status != oldStatus.Status ||
           newStatus.Progress != oldStatus.Progress)
          {
          std::cout << "job id " << jobs.at(i).id() << std::endl;
          if( newStatus.Status == remus::IN_PROGRESS)
            {
            std::cout << newStatus.Progress << std::endl;
            }
          else
            {
            std::cout << " status of job is: " << newStatus.Status << " " << remus::to_string(newStatus.Status)  << std::endl;
            }
          }

        //when the job has entered any of the finished states we remove it
        //from the jobs we are checking
        if( !newStatus.good() )
          {
          jobs.erase(jobs.begin()+i);
          js.erase(js.begin()+i);
          }
        }
      }
    }
  else
    {
    std::cout << "server doesn't support 2d meshes of raw triangles" << std::endl;
    }
  return 1;
}
