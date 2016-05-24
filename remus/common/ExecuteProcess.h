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

#ifndef remus_common__ExecuteProcess_h
#define remus_common__ExecuteProcess_h

#include <map>
#include <string>
#include <vector>

#include <remus/common/CommonExports.h>

#include <remus/common/CompilerInformation.h>
#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif


//forward declare the systools

namespace remus{
namespace common{


struct REMUSCOMMON_EXPORT ProcessPipe
{
  enum PipeType
    {
    None,
    STDIN,
    STDOUT,
    STDERR,
    Timeout
    };

  explicit ProcessPipe(ProcessPipe::PipeType pType):
    type(pType),text(){}

  bool valid() const { return type != None && type != Timeout; }

  ProcessPipe::PipeType type;
  std::string text;
};


class REMUSCOMMON_EXPORT ExecuteProcess
{
public:

  ExecuteProcess(const std::string& command, const std::vector<std::string>& args, const std::map<std::string,std::string>& env);
  ExecuteProcess(const std::string& command, const std::vector<std::string>& args);
  explicit ExecuteProcess(const std::string& command);

  //Will terminate the external process if has been started
  //and wasn't set to run in detached mode
  virtual ~ExecuteProcess();

  //Additional command and its args. The process's input pipe will be the output
  //pipe of the preceding process.
  void appendProcess(const std::string& command, const std::vector<std::string>& args);

  //Additional command. The process's input pipe will be the output pipe of the
  //preceding process.
  void appendProcess(const std::string& command);

  //execute the process.
  virtual void execute();

  //kills the process if running
  virtual bool kill();

  //Set whether the input pipe in the child is shared with the parent process.
  //The default is yes.
  void shareInPipeWithParent(bool choice)
  { return this->sharePipeWithParent(ProcessPipe::STDIN, choice); }

  //Set whether the output pipe in the child is shared with the parent process.
  //The default is no.
  void shareOutPipeWithParent(bool choice)
  { return this->sharePipeWithParent(ProcessPipe::STDOUT, choice); }

  //Set whether the error pipe in the child is shared with the parent process.
  //The default is no.
  void shareErrPipeWithParent(bool choice)
  { return this->sharePipeWithParent(ProcessPipe::STDERR, choice); }

  //Set input pipe to read from a file
  void inPipeFromFile(const std::string& filename)
  { return this->pipeToFile(ProcessPipe::STDIN, filename); }

  //Set output pipe to write to a file
  void outPipeToFile(const std::string& filename)
  { return this->pipeToFile(ProcessPipe::STDOUT, filename); }

  //Set error pipe to write to a file
  void errPipeToFile(const std::string& filename)
  { return this->pipeToFile(ProcessPipe::STDERR, filename); }

  //returns if the process is still alive
  bool isAlive();

  //returns if the process exited normally.
  //If the process is still running, killed, disowned, not yet running
  //this will return false
  bool exitedNormally();

  //Will poll for a given timeout value looking any output on the STDIN,STDOUT,and
  //STDERR streams.
  //The timeout's unit of time is SECONDS.
  //If the value of timeout is zero or greater we will wait that duration.
  //If the value of timeout is negative we will  block indefinitely until a
  //output on one of the pipes happens
  remus::common::ProcessPipe poll(double timeout);

private:
  ExecuteProcess(const ExecuteProcess&);
  void operator=(const ExecuteProcess&);

  //Set whether the given pipe in the child is shared with the parent process.
  //The default is no for Pipe_STDOUT and Pipe_STDERR and yes for Pipe_STDIN.
  void sharePipeWithParent(ProcessPipe::PipeType pipe, bool choice);

  //Set pipe output to a file
  void pipeToFile(ProcessPipe::PipeType pipe, const std::string& filename);

  struct Command
  {
    Command(const std::string& cmd, const std::vector<std::string>& args) :
      Cmd(cmd), Args(args) {}
    Command(const std::string& cmd) :
      Cmd(cmd), Args() {}
    std::string Cmd;
    std::vector<std::string> Args;
  };

  std::vector<Command> CommandQueue;
  std::map<std::string,std::string> Env;

  struct Process;
  Process* ExternalProcess;
};

}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif

#endif
