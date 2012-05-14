/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/worker/Worker.h>
#include <vector>
#include <string>

int main (int argc, char* argv[])
{
  //first argument is the server you want to connect too
  meshserver::worker::ServerConnection connection(argc,argv);
  meshserver::worker::Worker w(connection);

  meshserver::common::JobDetails jd = w.getJob();

  for(int progress=1; progress <= 100; ++progress)
    {
    if(progress%20==0)
#ifdef _WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
    meshserver::common::JobStatus status(jd.JobId,meshserver::IN_PROGRESS);
    status.Progress = progress;
    w.updateStatus(status);
    }

  meshserver::common::JobStatus status(jd.JobId,meshserver::FINISHED);
  w.updateStatus(status);

  meshserver::common::JobResult results(jd.JobId,"FAKE RESULTS");
  w.returnMeshResults(results);

  return 1;
}
