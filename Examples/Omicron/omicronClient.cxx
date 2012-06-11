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
  //construct a remus client that connects to the remus server
  //given the default server connection parameters.
  remus::client::ServerConnection conn;
  remus::Client c(conn);

  //if the server we connected to has a worker that supports 3d meshes
  //we will attempt to send a job.
  if(c.canMesh(remus::MESH3D))  //JobRequest constructor called implicitly.
    {
    //pass to the server a job request with a hardcodeded path to a file
    //as the data. This doesn't send the contents of the file to the server,
    //instead it send just the path. If you wanted to send the contents of the
    //file you need to fill an std::string with the contents.
    std::string omicronModel="smooth_surface_model.dat";
    remus::JobRequest request(remus::MESH3D,omicronModel);
    remus::Job job = c.submitJob(request);
    std::cout << "job submitted" << std::endl;

    //ask to see the status of our job we submitted
    remus::JobStatus oldStatus = c.jobStatus(job);
    std::cout << " status of job is: " << remus::to_string(oldStatus.Status)  << std::endl;

    while(oldStatus.Status != remus::FINISHED &&
          oldStatus.Status != remus::FAILED)
      {
      //keep asking for the status, we will print out to screen whenever the
      //state of the job changes, or the progress changes when the worker
      //is actually doing the task
      remus::JobStatus newStatus = c.jobStatus(job);
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


