/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal__ExternalProcess_h
#define __meshserver_broker_internal__ExternalProcess_h

#include <vector>
#include <string>

#include <kwtools/Process.h>
#include <kwtools/SystemTools.hxx>

namespace meshserver{
namespace common{

class ExecuteProcess
{
public:
  ExecuteProcess(const std::string& command, const std::vector<std::string>& args);
  ExecuteProcess(const std::string& command);

  virtual ~ExecuteProcess(){};

  //execute the process, state if you want the process to be detached
  //from the current process
  virtual void execute(const bool& deatched);

  //kills the process if running
  virtual bool kill();

private:
  launch();
  ExecuteProcess(const ExecuteProcess&);
  void operator=(const ExecuteProcess&);

  bool Alive;
  std::string Command;
  std::vector<std::string> Args;

  kwtoolsProcess ExternalProcess;
};

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(const std::string& command,
                               const std::vector<std::string>& args):
  Alive(false),Command(command),Args(arg),ExternalProcess()
  {

  }

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(const std::string& command):
  Alive(false),Command(command),Args(),ExternalProcess()
  {
    
  }

//-----------------------------------------------------------------------------  
void ExecuteProcess::execute(const bool& deatched)
{
  kwtoolsProcess_SetCommand(&this->ExternalProcess, this->Command);
  kwtoolsProcess_SetOption(&this->ExternalProcess, kwtoolsProcess_Option_HideWindow, true);
  kwtoolsProcess_SetOption(&this->ExternalProcess, kwtoolsProcess_Option_Detach, detached);
  kwtoolsProcess_Execute(&this->ExternalProcess);
  this->Alive = true;
}


//-----------------------------------------------------------------------------  
bool ExecuteProcess::kill()
{
  if(this->Alive && 
     vtksysProcess_GetState(this->ExternalProcess) == vtksysProcess_State_Executing)
    {
    vtksysProcess_Kill( this->ExternalProcess );
    vtksysProcess_WaitForExit(this->ExternalProcess, 0);
    }
  return false;
}

}
}
#endif
