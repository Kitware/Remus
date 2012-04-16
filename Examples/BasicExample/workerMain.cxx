/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/worker/Worker.h>
#include <vector>

int main (int argc, char* argv[])
{
  int timesToAskForJobs = 1;
  int jobsToAskForEachTime =1;

  //first argument is the number of times to as the server for jobs
  //the second argument is the number of jobs each time to ask for from the server
  //both default to 1
  if (argc == 3)
    {
    jobsToAskForEachTime = atoi(argv[2]);
    }
  if (argc >= 2)
    {
    timesToAskForJobs = atoi(argv[1]);
    }

  meshserver::worker::Worker w(meshserver::MESH2D);

  for(int i=0; i < timesToAskForJobs; ++i)
  {
    std::vector<meshserver::common::JobDetails> jobDetails;
    for(int j=0; j < jobsToAskForEachTime; ++j)
      {
      jobDetails.push_back(w.getJob());
      }

    for(int progress=1; progress <= 100; ++progress)
      {
      if(progress%20==0)
        sleep(1);
      for(int j=0; j < jobsToAskForEachTime; ++j)
        {
        meshserver::common::JobStatus status(jobDetails[j].JobId,meshserver::IN_PROGRESS);
        status.Progress = progress;
        w.updateStatus(status);
        }
      }

    for(int j=0; j < jobsToAskForEachTime; ++j)
      {
      meshserver::common::JobStatus status(jobDetails[j].JobId,meshserver::FINISHED);
      w.updateStatus(status);
      }

    for(int j=0; j < jobsToAskForEachTime; ++j)
      {
      meshserver::common::JobResult results(jobDetails[j].JobId,"FAKE RESULTS");
      w.returnMeshResults(results);
      }
  }

  return 1;
}
