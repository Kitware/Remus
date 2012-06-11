/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __omicron_worker_h
#define __omicron_worker_h

#include <remus/worker/Worker.h>
#include <vector>

namespace remus{ namespace common { class ExecuteProcess; } }

//simple struct that holds all the arguments to the omicron process
//that we are going to launch. This is filled by parsing the job object given
//to us by the server.
struct omicronSettings
{
  omicronSettings(remus::Job* details);
  std::vector<std::string> args;
};

//Construct
class OmicronWorker : public remus::worker::Worker
{
public:
  //construct a worker that can mesh a single type of mesh.
  //the connection object informs the worker where the server that holds the
  //jobs is currently running.
  //By default the Omicron Worker doesn't have a executable name set
  OmicronWorker(remus::MESH_TYPE type, remus::worker::ServerConnection const& conn);

  //will wait for the omicron process to close
  //before destroying self
  ~OmicronWorker();

  //Specify the name of the omicron mesher that we are using.
  //If you create an OmicronWorker for 2D meshing and you pass in the executable
  //name for a 3d mesher, undefined behaviour will occur.
  void setExecutableName(const std::string& name);

  //return the current executable name
  const std::string& executableName() const;

  //set the directory where we can find the given omicron mesher executable.
  void setExecutableDir(const std::string& path);
  const std::string& executableDir() const;

  //will launch an omicron process, if a process is currently
  //active it will block for the previous job to finish before starting
  void meshJob();

  //will halt a mesh job if one is in progress, by forcibly
  //terminating the omicron process
  bool terminateMeshJob();

protected:
  omicronSettings parseJobDetails();

  void launchOmicron();

  //waits for omicron to exit and then cleans up the OmicronProcess
  //Omicron process ptr will be NULL when this is finished
  void cleanlyExitOmicron();

  bool pollOmicronStatus();
  void updateProgress(int value);

  remus::Job* Job;
  remus::common::ExecuteProcess* OmicronProcess;
  std::string Name;
  std::string Directory;


};

#endif
