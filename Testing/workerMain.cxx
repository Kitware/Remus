/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/worker/Worker.h>

int main ()
{
  meshserver::worker::Worker w(meshserver::MESH2D);
  meshserver::common::JobDetails jd = w.getJob();

  meshserver::common::JobStatus status(jd.JobId,meshserver::IN_PROGRESS);
  for(int i=1; i <= 100; ++i)
    {
    status.Progress = i;
    if(i%20==0)
      sleep(1);
    w.updateStatus(status);
    }

  status = meshserver::common::JobStatus(jd.JobId,meshserver::FINISHED);
  w.updateStatus(status);

  meshserver::common::JobResult results(jd.JobId,"FAKE RESULTS");
  w.returnMeshResults(results);

  return 1;
}
