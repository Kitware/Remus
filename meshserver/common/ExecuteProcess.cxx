#include <meshserver/common/ExecuteProcess.h>

#include <sysTools/Process.h>

namespace meshserver{
namespace common{

//----------------------------------------------------------------------------
struct ExecuteProcess::Process
{
public:
  sysToolsProcess *Proc;
  bool Created;
  Process():Created(false)
    {
    this->Proc = sysToolsProcess_New();
    }
  ~Process()
    {
    if(this->Created)
      {
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
void ExecuteProcess::execute()
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
  -                            sysToolsProcess_Option_Detach, true);
  sysToolsProcess_Execute(this->ExternalProcess->Proc);

  this->ExternalProcess->Created = true;

  delete[] cmds;
}


//-----------------------------------------------------------------------------
bool ExecuteProcess::kill()
{
  if(this->ExternalProcess->Created &&
     sysToolsProcess_GetState(this->ExternalProcess->Proc) ==
     sysToolsProcess_State_Executing)
    {
    sysToolsProcess_Kill( this->ExternalProcess->Proc );
    sysToolsProcess_WaitForExit(this->ExternalProcess->Proc, 0);
    }
  return false;
}

//-----------------------------------------------------------------------------
bool ExecuteProcess::isAlive()
{
  if(!this->ExternalProcess->Created)
    {
    //never was created can't be  alive
    return false;
    }
  int state = sysToolsProcess_GetState(this->ExternalProcess->Proc);
  return state == sysToolsProcess_State_Executing;
}


}
}
