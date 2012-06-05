/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"

#include <remus/common/ExecuteProcess.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <iostream>

//-----------------------------------------------------------------------------
omicronSettings::omicronSettings(remus::JobDetails *details):
  args()
{
  //job details are the arguments for the omicron instance we have
  //dirty hack since we know that the client is only passing one argument to omicron
  args.push_back(details->Path);
}

//-----------------------------------------------------------------------------
OmicronWorker::OmicronWorker(const remus::worker::ServerConnection &conn):
  remus::worker::Worker(remus::MESH3D,conn),
  JobDetails(NULL),
  OmicronProcess(NULL),
  Name("model"),
  Directory(boost::filesystem::current_path().string())
{

}

//-----------------------------------------------------------------------------
OmicronWorker::~OmicronWorker()
{
  if(this->JobDetails)
    {
    delete this->JobDetails;
    }
  this->cleanlyExitOmicron();
}

//-----------------------------------------------------------------------------
void OmicronWorker::setExecutableName(const std::string& name)
{
  this->Name = name;
}

//-----------------------------------------------------------------------------
const std::string& OmicronWorker::executableName() const
{
  return this->Name;
}

//-----------------------------------------------------------------------------
void OmicronWorker::setExecutableDir(const std::string& path)
{
  this->Directory = path;
}

//-----------------------------------------------------------------------------
const std::string& OmicronWorker::executableDir() const
{
  return this->Directory;
}

//-----------------------------------------------------------------------------
void OmicronWorker::meshJob()
{
  if(this->JobDetails)
    {
    delete this->JobDetails;
    this->JobDetails = NULL;
    }

  this->JobDetails = new remus::JobDetails(this->getJob());
  this->launchOmicron( );

  //poll on omicron now
  bool valid = this->pollOmicronStatus();
  if(valid)
    {
    //send to the server the mesh results too
    remus::JobResult results(this->JobDetails->JobId,
                                          "FAKE RESULTS");
    this->returnMeshResults(results);
    }

  this->cleanlyExitOmicron();
}

//-----------------------------------------------------------------------------
bool OmicronWorker::terminateMeshJob()
{
  if(this->OmicronProcess)
    {
    //update the server with the fact that will had to kill the job
    remus::JobStatus status(this->JobDetails->JobId,
                                         remus::FAILED);
    this->updateStatus(status);

    this->OmicronProcess->kill();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void OmicronWorker::launchOmicron()
{
  //wait for any current process to finish before starting new one
  this->cleanlyExitOmicron();

  omicronSettings settings(this->JobDetails);
  boost::filesystem::path executionPath = this->Directory;
  executionPath /= this->Name;
#ifdef WIN32
  executionPath.replace_extension(".exe");
#endif

  this->OmicronProcess = new remus::common::ExecuteProcess(
                           executionPath.string(),settings.args);

  //actually launch the new process
  this->OmicronProcess->execute(false);
}

//-----------------------------------------------------------------------------
void OmicronWorker::cleanlyExitOmicron()
{
  //waits for omicron to exit and then cleans up the OmicronProcess
  //Omicron process ptr will be NULL when this is finished
  if(this->OmicronProcess)
    {
    delete this->OmicronProcess;
    this->OmicronProcess = NULL;
    }
}

//-----------------------------------------------------------------------------
bool OmicronWorker::pollOmicronStatus()
{
  //loop on polling of the omicron process
  typedef remus::common::ProcessPipe ProcessPipe;

  //poll on STDOUT and STDERRR only
  bool validExection=true;
  int status = 0;
  while(this->OmicronProcess->isAlive()&& validExection )
    {
    //poll till we have a data, waiting for-ever!
    ProcessPipe data = this->OmicronProcess->poll(-1);
    if(data.type == ProcessPipe::STDOUT)
      {
      //we have something on the output pipe
      std::size_t pos = data.text.find("Progress: ");
      if(pos != std::string::npos)
        {
        //omicron doesn't actually report back precentage done,
        //instead it reports back just messages. It will generate 10 progress messages
        //during the execution so use that as a yardstick
        status += 10;
        this->updateProgress(status);
        }

      //for now also dump it to the console
      std::cout << data.text << std::endl;

      }
    else if(data.type == ProcessPipe::STDERR)
      {
      //we have data on std err which in omicron case is bad, so terminate
      //the job to make sure the mesh is reported to have incorrectly meshed
      validExection = false;
      }
    else
      {
      std::cout << "No Poll" << std::endl;
      }
    }

  //verify we exited normally, not segfault or numeric exception
  validExection &= this->OmicronProcess->exitedNormally();

  if(!validExection)
    {//we call terminate to make sure we send the message to the server
    //that we have failed to mesh the input correctly
    this->terminateMeshJob();
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
void OmicronWorker::updateProgress(int value)
{
  remus::JobStatus status(this->JobDetails->JobId,
                                       remus::IN_PROGRESS);
  status.Progress = value;
  this->updateStatus(status);
}
