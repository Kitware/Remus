/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"
#include <meshserver/common/ExecuteProcess.h>
#include <boost/filesystem.hpp>


namespace {

int progress_value(const std::string& p)
{
  return atoi(p.c_str());
}

}

//-----------------------------------------------------------------------------
omicronSettings::omicronSettings(meshserver::common::JobDetails *details):
  exec(),args()
{
  //parse the details into a executable and a collection of arguments

}

//-----------------------------------------------------------------------------
OmicronWorker::OmicronWorker():
  meshserver::worker::Worker(meshserver::MESH3D),
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

  this->JobDetails = new meshserver::common::JobDetails(this->getJob());
  this->launchOmicron( );

  //poll on omicron now
  bool valid = this->pollOmicronStatus();
  if(valid)
    {
    //send to the broker the mesh results too
    meshserver::common::JobResult results(this->JobDetails->JobId,
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
    //update the broker with the fact that will had to kill the job
    meshserver::common::JobStatus status(this->JobDetails->JobId,
                                         meshserver::FAILED);
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
  this->OmicronProcess = new meshserver::common::ExecuteProcess(settings.exec,
                                                                settings.args);

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
  typedef meshserver::common::ExecuteProcess::PolledPipes PolledPipes;
  typedef meshserver::common::ExecuteProcess::PipeData PipeData;

  //poll on STDOUT and STDERRR only
  PolledPipes items(PolledPipes::STDOUT,PolledPipes::STDERR);

  bool noErrorOutput=true;
  while(this->OmicronProcess->isAlive()&&noErrorOutput)
    {
    //poll till we have a data, waiting for-ever!
    PipeData data = this->OmicronProcess->poll(items,0);
    if(data.type() == PollItems::STDOUT)
      {
      //we have something on stdOUT
      std::size_t pos = data.text().find("Progress:");
      if(pos != std::string::npos)
        {
        //get a subsection of the string with the progress value
        int status=progress_value(std::string(data.text(),pos,data.size()-pos));
        this->updateProgress(status);
        }
      }
    if(data.type() == PollItems::STDERR)
      {
      //we have data on std err which in omicron case is bad, so terminate
      noErrorOutput = false;
      }
    }
  if(!noErrorOutput)
    {
    this->terminateMeshJob();
    return false;
    }

  //update the broker with the fact that will have finish the job
  meshserver::common::JobStatus status(this->JobDetails->JobId,
                                       meshserver::FINISHED);
  this->updateStatus(status);
  return true;
}

//-----------------------------------------------------------------------------
void OmicronWorker::updateProgress(int value)
{
  meshserver::common::JobStatus status(this->JobDetails->JobId,
                                       meshserver::IN_PROGRESS);
  status.Progress = value;
  this->updateStatus(status);
}
