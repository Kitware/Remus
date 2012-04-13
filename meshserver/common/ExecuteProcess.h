/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common__ExternalProcess_h
#define __meshserver_common__ExternalProcess_h

#include <vector>
#include <string>

//forward declare the systools

namespace meshserver{
namespace common{

class ExecuteProcess
{
public:
  ExecuteProcess(const std::string& command, const std::vector<std::string>& args);
  explicit ExecuteProcess(const std::string& command);

  //Will terminate the external process if has been started
  //and wasn't set to run in detached mode
  virtual ~ExecuteProcess();

  //execute the process
  virtual void execute();

  //kills the process if running
  virtual bool kill();

  //returns if the process is still alive
  bool isAlive();

private:
  ExecuteProcess(const ExecuteProcess&);
  void operator=(const ExecuteProcess&);

  std::string Command;
  std::vector<std::string> Args;

  struct Process;
  Process* ExternalProcess;
};

}
}
#endif
