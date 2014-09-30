/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/worker/Worker.h>
#include <remus/common/SleepFor.h>

#include <vector>
#include <string>
#include <iostream>

int main (int argc, char* argv[])
{
  using namespace remus::meshtypes;
  using namespace remus::proto;


  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
    }

  remus::common::MeshIOType io_type =
                    remus::common::make_MeshIOType(Edges(),Mesh2D());
  JobRequirements requirements = make_JobRequirements(io_type,
                                                            "InfiniteWorker",
                                                            "");
  remus::Worker w(requirements,connection);

  while(true)
  {
    remus::worker::Job jd = w.getJob();
    switch(jd.validityReason())
    {
      case remus::worker::Job::INVALID:
        return 1;
      case remus::worker::Job::TERMINATE_WORKER:
        return 1;
      default:
        break;
    }

    JobProgress jprogress;
    JobStatus status(jd.id(),remus::IN_PROGRESS);
    for(int progress=1; progress <= 100; ++progress)
      {
      if(progress%20==0)
        {
        remus::common::SleepForMillisec(1000);
        jprogress.setValue(progress);
        jprogress.setMessage("Random Content From InfiniteWorker");
        status.updateProgress(jprogress);
        w.updateStatus(status);
        }
      }

    JobResult results = remus::proto::make_JobResult(jd.id(),
                                                 "Hello From InfiniteWorker");
    w.returnResult(results);
  }

  return 0;
}
