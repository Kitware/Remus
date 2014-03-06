#include <cstdio>

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include "TetGenInput.h"
#include "TetGenResult.h"

#include <iostream>

int main (int argc, char* argv[])
{
  remus::client::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::client::make_ServerConnection(std::string(argv[1]));
    }

  //create a client object that will connect to the default remus server
  remus::Client c(connection);

  TetGenInput input_data("pmdc.poly");
  remus::proto::JobContent content =
      remus::proto::make_JobContent(input_data);

  remus::common::MeshIOType mtype( (remus::meshtypes::PiecewiseLinearComplex()),
                                   (remus::meshtypes::Mesh2D()) );
  remus::proto::JobRequirements request =
      remus::proto::make_JobRequirements( mtype, "TetGenWorker", "");

  if(c.canMesh(mtype))
    {
    remus::proto::JobSubmission sub(request);
    sub["data"]=content;

    std::cout << "submitting tetgen job" << std::endl;
    remus::proto::Job job = c.submitJob(sub);
    remus::proto::JobStatus jobState = c.jobStatus(job);

    //wait while the job is running
    while(jobState.good())
      {
      jobState = c.jobStatus(job);
      };

    if(jobState.finished())
      {
      std::cout << "tetgen finished meshing" << std::endl;
      remus::proto::JobResult result = c.retrieveResults(job);
      TetGenResult tetgen_data(result);

      //todo rip out the data and verify we have the correct resulting mesh
      }
    }
  return 0;
}