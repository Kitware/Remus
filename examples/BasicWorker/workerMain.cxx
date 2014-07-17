/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/worker/Worker.h>
#include <remus/testing/Testing.h>

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
                                                            "BasicWorker",
                                                            "");
  remus::Worker w(requirements,connection);
  remus::worker::Job jd = w.getJob();

  const remus::proto::JobContent& content =
                                    jd.submission().find("data")->second;

  JobProgress jprogress;
  JobStatus status(jd.id(),remus::IN_PROGRESS);

  for(int progress=1; progress <= 100; ++progress)
    {
    if(progress%20==0)
      {
      remus::testing::sleepForMillisec(1000);

      jprogress.setValue(progress);
      jprogress.setMessage("Example Message With Random Content");
      status.updateProgress(jprogress);
      w.updateStatus( status );
      }
    }

  //respond by modifying the job content
  std::string result(content.data(),content.dataSize());
  result += " and Hello Client";
  remus::proto::JobResult results =
                                  remus::proto::make_JobResult(jd.id(),result);
  w.returnMeshResults(results);

  return 1;
}
