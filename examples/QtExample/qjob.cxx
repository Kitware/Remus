
#include "qjob.h"

#include <remus/client/Client.h>
#include <iostream>

//-----------------------------------------------------------------------------
void qjob::run()
{
  using namespace remus::meshtypes;
  using namespace remus::proto;

  std::cerr << "spawning a job" << std::endl;
  //create a default server connection
  remus::client::ServerConnection conn;
  remus::client::Client c(conn);

  remus::common::MeshIOType io_type((Model()),(Mesh3D()));

  JobRequirements reqs = make_JobRequirements(io_type, "BasicWorker", "");

  remus::proto::JobSubmission sub(reqs);

  remus::proto::Job job = c.submitJob(sub);
  remus::proto::JobStatus jobState = c.jobStatus(job);
  while(jobState.good())
    {
    jobState = c.jobStatus(job);
    };

  if(jobState.failed())
    {
    std::cerr << "job failed" << std::endl;
    std::cerr << remus::to_string(jobState.status()) << std::endl;
    }

  if(jobState.finished())
    {
    remus::proto::JobResult result = c.retrieveResults(job);
    std::cerr << "job finished" << std::endl;
    }

}
