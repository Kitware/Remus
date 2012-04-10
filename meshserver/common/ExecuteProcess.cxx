#include <meshserver/common/ExecuteProcess.h>

#include <sysTools/Process.h>

namespace meshserver{
namespace common{

//----------------------------------------------------------------------------
struct ExecuteProcess::Process
{
public:
  sysToolsProcess *Proc;
  bool Alive;
  bool Detached;
  Process():Alive(false),Detached(false)
    {
    this->Proc = sysToolsProcess_New();
    }
  ~Process()
    {
    if(this->Alive && !this->Detached)
      {
      //if the process isn't detached we should kill it too when we leave
      sysToolsProcess_Delete(this->Proc);
      }

    }
};

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(const std::string& command,
                               const std::vector<std::string>& args):
  Command(command),
  Args(args)
  {
  this->ExternalProcess = new ExecuteProcess::Process();
  }

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(const std::string& command):
  Command(command),
  Args()
  {
  this->ExternalProcess = new ExecuteProcess::Process();
  }

//-----------------------------------------------------------------------------
ExecuteProcess::~ExecuteProcess()
{
  delete this->ExternalProcess;
}

//-----------------------------------------------------------------------------
void ExecuteProcess::execute(const bool& detached)
{
  //allocate array large enough for command str, args, and null entry
  const std::size_t size(this->Args.size() + 2);
  const char **cmds = new const char * [size];
  cmds[0] = this->Command.c_str();
  for(std::size_t i=0; i < this->Args.size();++i)
    {
    cmds[1 + i] = this->Args[i].c_str();
    }
  cmds[size-1]=NULL;



  sysToolsProcess_SetCommand(this->ExternalProcess->Proc, cmds);
  sysToolsProcess_SetOption(this->ExternalProcess->Proc,
                            sysToolsProcess_Option_HideWindow, true);
  sysToolsProcess_SetOption(this->ExternalProcess->Proc,
                            sysToolsProcess_Option_Detach, detached);
  sysToolsProcess_Execute(this->ExternalProcess->Proc);

  this->ExternalProcess->Alive = true;
  this->ExternalProcess->Detached = detached;

  delete[] cmds;
}


//-----------------------------------------------------------------------------
bool ExecuteProcess::kill()
{
  if(this->ExternalProcess->Alive &&
     sysToolsProcess_GetState(this->ExternalProcess->Proc) ==
     sysToolsProcess_State_Executing)
    {
    sysToolsProcess_Kill( this->ExternalProcess->Proc );
    sysToolsProcess_WaitForExit(this->ExternalProcess->Proc, 0);
    }
  return false;
}


}
}
