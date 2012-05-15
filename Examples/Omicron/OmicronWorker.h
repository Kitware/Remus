/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __omicron_worker_h
#define __omicron_worker_h

#include <meshserver/worker/Worker.h>
#include <vector>

namespace meshserver{ namespace common { class ExecuteProcess; } }

struct omicronSettings
{
  omicronSettings(meshserver::common::JobDetails* details);
  std::vector<std::string> args;
};

class OmicronWorker : public meshserver::worker::Worker
{
public:
  //construct a worker that can mesh a single type
  //give it a zeroMQ endpoint to connect too
  explicit OmicronWorker(meshserver::worker::ServerConnection const& conn);

  //will wait for the omicron process to close
  //before destroying self
  ~OmicronWorker();

  void setExecutableName(const std::string& name);
  const std::string& executableName() const;

  void setExecutableDir(const std::string& path);
  const std::string& executableDir() const;

  //will launch an omicron process, if a process is currently
  //active it will block for the previous job to finish before starting
  void meshJob();

  //will halt a mesh job if one is in progress, by forcibile
  //terminate the omicron process
  bool terminateMeshJob();

protected:
  omicronSettings parseJobDetails();

  void launchOmicron();

  //waits for omicron to exit and then cleans up the OmicronProcess
  //Omicron process ptr will be NULL when this is finished
  void cleanlyExitOmicron();

  bool pollOmicronStatus();
  void updateProgress(int value);

  meshserver::common::JobDetails* JobDetails;
  meshserver::common::ExecuteProcess* OmicronProcess;
  std::string Name;
  std::string Directory;


};

#endif
