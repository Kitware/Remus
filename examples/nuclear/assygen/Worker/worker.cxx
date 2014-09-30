//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "worker.h"
#include "AssygenInput.h"
#include "AssygenOutput.h"

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>

#include <remus/proto/JobStatus.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

//----------------------------------------------------------------------------
worker::worker( remus::worker::ServerConnection const& conn )
  :remus::worker::Worker(
    remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::SceneFile(),
                                     remus::meshtypes::Model()),
      "AssyGenWorker",
      ""),
    conn),
   Process(NULL)
{
}

worker::~worker()
{
  cleanlyExit();
}

//----------------------------------------------------------------------------
void worker::meshJob()
{
  remus::worker::Job j = this->getJob();

  AssygenInput in(j);
  std::cout << (std::string)(in) << std::endl;

  launchProcess(in);

  if (!pollStatus(j))
  {
    this->jobFailed(j);
    return;
  }

  //Do it
  remus::proto::JobResult results = remus::proto::make_JobResult(j.id(),
                                                               in.getPrefix());
  this->returnResult(results);

  return;
}

//----------------------------------------------------------------------------
void worker::jobFailed(const remus::worker::Job& job)
{
  remus::proto::JobStatus status(job.id(),remus::FAILED);
  this->updateStatus(status);

  return;
}

void worker::launchProcess(const AssygenInput& job)
{
  cleanlyExit();
  //save the current path
  boost::filesystem::path cwd = boost::filesystem::current_path();

  //Run in the output file incase of relative locations
  boost::filesystem::path filePath = boost::filesystem::absolute(job.getPrefix()+".inp");
  boost::filesystem::current_path(filePath.parent_path());

  std::cout << "RUNNING " << job.getExecutablePath() << " " << job.getPrefix() << std::endl;
  //make a cleaned up path with no relative
  boost::filesystem::path executePath =
      boost::filesystem::absolute(job.getExecutablePath());
  std::vector<std::string> args;
  args.push_back(job.getPrefix());
  this->Process =
      new remus::common::ExecuteProcess( executePath.string(), args);

  //actually launch the new process
  this->Process->execute(remus::common::ExecuteProcess::Attached);

  //move back to the proper directory
  boost::filesystem::current_path(cwd);
}

void worker::cleanlyExit()
{
  if(this->Process)
  {
    delete this->Process;
    this->Process = NULL;
  }
}

//-----------------------------------------------------------------------------
bool worker::pollStatus(const remus::worker::Job& job)
{
  //loop on polling of the omicron process
  typedef remus::common::ProcessPipe ProcessPipe;

  //poll on STDOUT and STDERRR only
  bool validExection=true;
  remus::proto::JobStatus status(job.id(),remus::IN_PROGRESS);
  while(this->Process->isAlive()&& validExection )
  {
    //poll till we have a data, waiting for-ever!
    ProcessPipe data = this->Process->poll(-1);
    if(data.type == ProcessPipe::STDOUT)
    {
      //we have something on the output pipe
      remus::proto::JobProgress progress(data.text);
      status.updateProgress(progress);
      this->updateStatus(status);
    }
  }

  //verify we exited normally, not segfault or numeric exception
  validExection &= this->Process->exitedNormally();

  if(!validExection)
  {//we call terminate to make sure we send the message to the server
    //that we have failed to mesh the input correctly
    this->cleanlyExit();
    return false;
  }
  return true;
}

