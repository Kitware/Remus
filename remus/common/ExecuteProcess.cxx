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

#include <RemusSysTools/Process.h>

#include <stdlib.h>

namespace{

remus::common::ProcessPipe::PipeType toProcessPipeType(int type)
  {
  typedef remus::common::ProcessPipe PPipe;
  typedef remus::common::ProcessPipe::PipeType PipeType;

  RemusSysToolsProcess_Pipes_e processType =
      static_cast<RemusSysToolsProcess_Pipes_e>(type);
  PipeType pipeType;
  switch(processType)
    {
    case RemusSysToolsProcess_Pipe_STDIN:
      pipeType = PPipe::STDIN;
      break;
    case RemusSysToolsProcess_Pipe_STDOUT:
      pipeType = PPipe::STDOUT;
      break;
    case RemusSysToolsProcess_Pipe_STDERR:
      pipeType = PPipe::STDERR;
      break;
    case RemusSysToolsProcess_Pipe_Timeout:
      pipeType = PPipe::Timeout;
      break;
    case RemusSysToolsProcess_Pipe_None:
    default:
      pipeType = PPipe::None;
      break;
    }
  return pipeType;
  }

int fromProcessPipeType(remus::common::ProcessPipe::PipeType type)
  {
  typedef remus::common::ProcessPipe PPipe;

  RemusSysToolsProcess_Pipes_e pipeType;
  switch(type)
    {
    case PPipe::STDIN:
      pipeType = RemusSysToolsProcess_Pipe_STDIN;
      break;
    case PPipe::STDOUT:
      pipeType = RemusSysToolsProcess_Pipe_STDOUT;
      break;
    case PPipe::STDERR:
      pipeType = RemusSysToolsProcess_Pipe_STDERR;
      break;
    case PPipe::Timeout:
      pipeType = RemusSysToolsProcess_Pipe_Timeout;
      break;
    case PPipe::None:
    default:
      pipeType = RemusSysToolsProcess_Pipe_None;
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
  RemusSysToolsProcess *Proc;
  bool Created;
  Process():Created(false)
    {
    this->Proc = RemusSysToolsProcess_New();
    }
  ~Process()
    {
    if(this->Created)
      {
      RemusSysToolsProcess_Delete(this->Proc);
      }

    }
};

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(
  const std::string& command,
  const std::vector<std::string>& args,
  const std::map<std::string,std::string>& env):
  CommandQueue(1,ExecuteProcess::Command(command,args)),
  Env(env)
{
  this->ExternalProcess = new ExecuteProcess::Process();
}

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(const std::string& command,
                               const std::vector<std::string>& args):
  CommandQueue(1,ExecuteProcess::Command(command,args))
  {
  this->ExternalProcess = new ExecuteProcess::Process();
  }

//-----------------------------------------------------------------------------
ExecuteProcess::ExecuteProcess(const std::string& command):
  CommandQueue(1,ExecuteProcess::Command(command))
  {
  this->ExternalProcess = new ExecuteProcess::Process();
  }

//-----------------------------------------------------------------------------
ExecuteProcess::~ExecuteProcess()
{
  delete this->ExternalProcess;
}

//-----------------------------------------------------------------------------
void ExecuteProcess::appendProcess(const std::string& command,
                                   const std::vector<std::string>& args)
{
  this->CommandQueue.push_back(ExecuteProcess::Command(command,args));
}

//-----------------------------------------------------------------------------
void ExecuteProcess::appendProcess(const std::string& command)
{
  this->CommandQueue.push_back(ExecuteProcess::Command(command));
}

//-----------------------------------------------------------------------------
void ExecuteProcess::execute()
{
  for (std::size_t cmdId=0;cmdId<this->CommandQueue.size();cmdId++)
  {
    //allocate array large enough for command str, args, and null entry
    const std::size_t size(this->CommandQueue[cmdId].Args.size() + 2);
    const char **cmds = new const char * [size];
    cmds[0] = this->CommandQueue[cmdId].Cmd.c_str();
    for(std::size_t i=0; i < this->CommandQueue[cmdId].Args.size();++i)
    {
      cmds[1 + i] = this->CommandQueue[cmdId].Args[i].c_str();
    }
    cmds[size-1]=NULL;

    if (cmdId == 0)
    {
      RemusSysToolsProcess_SetCommand(this->ExternalProcess->Proc,cmds);
    }
    else
    {
      RemusSysToolsProcess_AddCommand(this->ExternalProcess->Proc,cmds);
    }
    delete[] cmds;
  }

  // For each requested environment variable, save
  // the old value before setting the new one.
  typedef std::map<std::string,std::string> envmap_t;
  envmap_t TmpEnv;
  for (envmap_t::const_iterator it = this->Env.begin(); it != this->Env.end(); ++it)
    {
    char* buf;
#if !defined(_WIN32) || defined(__CYGWIN__)
    buf = getenv(it->first.c_str());
    if (buf && buf[0])
      TmpEnv[it->first] = buf;
    setenv(it->first.c_str(), it->second.c_str(), 1);
#else
    const bool valid = (_dupenv_s(&buf, NULL, it->first.c_str()) == 0) &&
                       (buf != NULL);
    if (valid)
      TmpEnv[it->first] = buf;
    _putenv_s(it->first.c_str(), it->second.c_str());
#endif
    }

  RemusSysToolsProcess_SetOption(this->ExternalProcess->Proc,
                            RemusSysToolsProcess_Option_HideWindow, true);

  RemusSysToolsProcess_Execute(this->ExternalProcess->Proc);

  // Now that the process has been created, reset the environment.
  for (envmap_t::const_iterator it = TmpEnv.begin(); it != TmpEnv.end(); ++it)
    {
#if !defined(_WIN32) || defined(__CYGWIN__)
    setenv(it->first.c_str(), it->second.c_str(), 1);
#else
    _putenv_s(it->first.c_str(), it->second.c_str());
#endif
    }

  this->ExternalProcess->Created = true;
}


//-----------------------------------------------------------------------------
bool ExecuteProcess::kill()
{
  if(this->ExternalProcess->Created &&
     RemusSysToolsProcess_GetState(this->ExternalProcess->Proc) ==
     RemusSysToolsProcess_State_Executing)
    {
    RemusSysToolsProcess_Kill( this->ExternalProcess->Proc );
    RemusSysToolsProcess_WaitForExit(this->ExternalProcess->Proc, 0);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void ExecuteProcess::sharePipeWithParent(ProcessPipe::PipeType pipe,bool choice)
{
  RemusSysToolsProcess_SetPipeShared(this->ExternalProcess->Proc,
                                     fromProcessPipeType(pipe),
                                     choice);
}

//-----------------------------------------------------------------------------
void ExecuteProcess::pipeToFile(ProcessPipe::PipeType pipe,
                                const std::string& filename)
{
  RemusSysToolsProcess_SetPipeFile(this->ExternalProcess->Proc,
                                   fromProcessPipeType(pipe),
                                   filename.c_str());
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
  RemusSysToolsProcess_WaitForExit(this->ExternalProcess->Proc, &timeout);

  int state = RemusSysToolsProcess_GetState(this->ExternalProcess->Proc);
  return state == RemusSysToolsProcess_State_Executing;
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
  RemusSysToolsProcess_WaitForExit(this->ExternalProcess->Proc, &timeout);

  int state = RemusSysToolsProcess_GetState(this->ExternalProcess->Proc);
  return (state == RemusSysToolsProcess_State_Exited);
}



//-----------------------------------------------------------------------------
remus::common::ProcessPipe ExecuteProcess::poll(double timeout)
{
  //The timeout's unit of time is SECONDS.

  typedef remus::common::ProcessPipe PPipe;

  if(!this->ExternalProcess->Created)
    {
    return PPipe(PPipe::None);
    }

  //convert our syntax for timout to the RemusSysTools version
  //our negative values mean zero for sysToolProcess
  //our zero value means a negative value
  //other wise we are the same

  //RemusSysTools currently doesn't have a block for inifinte time that acutally works
  const double fakeInfiniteWait = 100000000;
  double realTimeOut = (timeout == 0 ) ? -1 : ( timeout < 0) ? fakeInfiniteWait : timeout;

  //poll sys tool for data
  int length;
  char* data;
  int pipe = RemusSysToolsProcess_WaitForData(this->ExternalProcess->Proc,
                                          &data,&length,&realTimeOut);
  PPipe::PipeType type = toProcessPipeType(pipe);

  PPipe result(type);
  if(result.valid())
    {
    result.text = std::string(data,length);
    }
  return result;
}


}
}
