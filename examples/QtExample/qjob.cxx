
#include "qjob.h"

#include <remus/client/Client.h>
#include <iostream>

//-----------------------------------------------------------------------------
void qjob::run()
{
  std::cerr << "spawning a job" << std::endl;
  //create a default server connection
  remus::client::ServerConnection conn;
  remus::client::Client c(conn);

  remus::client::JobRequest reqs(remus::meshtypes::Model(),
                                 remus::meshtypes::Mesh3D(),
                                 "LETS KILL THIS BAD BOY");

  remus::client::Job job = c.submitJob(reqs);

  while(!job.valid())
    {
    job = c.submitJob(reqs);
    }

  remus::client::JobStatus jobState = c.jobStatus(job);

  while(jobState.good())
    {
    jobState = c.jobStatus(job);
    };

  if(jobState.failed())
    {
    std::cerr << "job failed" << std::endl;
    std::cerr << remus::to_string(jobState.Status) << std::endl;
    std::cerr << remus::to_string(jobState.Status) << std::endl;
    }

  if(jobState.finished())
    {
    remus::client::JobResult result = c.retrieveResults(job);
    std::cerr << "job finished" << std::endl;
    }

}
