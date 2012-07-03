/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/worker/Worker.h>
#include <vector>
#include <string>

int main (int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::ServerConnection(std::string(argv[1]));
    }

  remus::Worker w(remus::MESH2D,connection);

  remus::Job jd = w.getJob();

  remus::JobStatus status(jd.id(),remus::IN_PROGRESS);
  for(int progress=1; progress <= 100; ++progress)
    {
    if(progress%20==0)
      {
#ifdef _WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
      status.Progress.setValue(progress);
      w.updateStatus(status);
      }
    }

  //mark the status as finished now, and resend.
  status.Status = remus::FINISHED;
  w.updateStatus(status);

  remus::JobResult results(jd.id(),"FAKE RESULTS");
  w.returnMeshResults(results);

  return 1;
}
