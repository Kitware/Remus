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

  //we create a basic job request for a mesh2d job, with the data contents of "TEST"
  remus::client::JobRequest request(remus::PIECWISE_LINEAR_COMPLEX,
                                    remus::MESH3D,
                                    input_data);

  if(c.canMesh(request))
    {
    std::cout << "submitting tetgen job" << std::endl;
    remus::client::Job job = c.submitJob(request);
    remus::client::JobStatus jobState = c.jobStatus(job);

    //wait while the job is running
    while(jobState.good())
      {
      jobState = c.jobStatus(job);
      };

    if(jobState.finished())
      {
      std::cout << "tetgen finished meshing" << std::endl;
      remus::client::JobResult result = c.retrieveResults(job);
      TetGenResult tetgen_data(result);

      //todo rip out the data and verify we have the correct resulting mesh
      }
    }
  return 0;
}