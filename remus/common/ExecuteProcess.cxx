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

#include <remus/common/ExecuteProcess.h>

#include <sysTools/Process.h>

namespace{

remus::common::ProcessPipe::PipeType typeToType(int type)
  {
  typedef remus::common::ProcessPipe PPipe;
  typedef remus::common::ProcessPipe::PipeType PipeType;

  sysToolsProcess_Pipes_e processType =
      static_cast<sysToolsProcess_Pipes_e>(type);
  PipeType pipeType;
  switch(processType)
    {
    case sysToolsProcess_Pipe_STDIN:
      pipeType = PPipe::STDIN;
      break;
    case sysToolsProcess_Pipe_STDOUT:
      pipeType = PPipe::STDOUT;
      break;
    case sysToolsProcess_Pipe_STDERR:
      pipeType = PPipe::STDERR;
      break;
    case sysToolsProcess_Pipe_Timeout:
      pipeType = PPipe::Timeout;
      break;
    case sysToolsProcess_Pipe_None:
    default:
      pipeType = PPipe::None;
      break;
    }
  return pipeType;
  }
}

namespace remus{
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
void ExecuteProcess::execute(bool detachProcess)
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
                              sysToolsProcess_Option_Detach, detachProcess);
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
  //poll just to see if the state has changed
  double timeout = -1; //set to a negative number to poll
  sysToolsProcess_WaitForExit(this->ExternalProcess->Proc, &timeout);

  int state = sysToolsProcess_GetState(this->ExternalProcess->Proc);
  return state == sysToolsProcess_State_Executing;
}

//-----------------------------------------------------------------------------
bool ExecuteProcess::exitedNormally()
{
  if(!this->ExternalProcess->Created)
    {
    //never was created can't be  alive
    return false;
    }
  //poll just to see if the state has changed
  double timeout = -1; //set to a negative number to poll
  sysToolsProcess_WaitForExit(this->ExternalProcess->Proc, &timeout);

  int state = sysToolsProcess_GetState(this->ExternalProcess->Proc);
  return (state == sysToolsProcess_State_Exited);
}



//-----------------------------------------------------------------------------
remus::common::ProcessPipe ExecuteProcess::poll(double timeout)
{
  //The timeout's unit of time is SECONDS.

  typedef remus::common::ProcessPipe ProcessPipe;

  if(!this->ExternalProcess->Created)
    {
    return ProcessPipe(ProcessPipe::None);
    }

  //convert our syntax for timout to the systools version
  //our negative values mean zero for sysToolProcess
  //our zero value means a negative value
  //other wise we are the same

  //systools currently doesn't have a block for inifinte time that acutally works
  const double fakeInfiniteWait = 100000000;
  double realTimeOut = (timeout == 0 ) ? -1 : ( timeout < 0) ? fakeInfiniteWait : timeout;

  //poll sys tool for data
  int length;
  char* data;
  int pipe = sysToolsProcess_WaitForData(this->ExternalProcess->Proc,
                                          &data,&length,&realTimeOut);
  ProcessPipe::PipeType type = typeToType(pipe);

  ProcessPipe result(type);
  if(result.valid())
    {
    result.text = std::string(data,length);
    }
  return result;
}


}
}
