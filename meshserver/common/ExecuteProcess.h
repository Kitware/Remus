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

#ifndef __meshserver_common__ExecuteProcess_h
#define __meshserver_common__ExecuteProcess_h

#include <set>
#include <string>
#include <vector>

//forward declare the systools

namespace meshserver{
namespace common{


struct ProcessPipe
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


class ExecuteProcess
{
public:
  ExecuteProcess(const std::string& command, const std::vector<std::string>& args);
  explicit ExecuteProcess(const std::string& command);

  //Will terminate the external process if has been started
  //and wasn't set to run in detached mode
  virtual ~ExecuteProcess();

  //execute the process. set detach to true if you don't want to recieve
  //any output from the child process. Be sure not to poll on a detached
  //process as it won't work
  virtual void execute(bool detachProcess);

  //kills the process if running
  virtual bool kill();

  //returns if the process is still alive
  bool isAlive();

  //returns if the process exited normally.
  //If the process is still running, killed, disowned, not yet running
  //this will return false
  bool exitedNormally();

  //Will poll for a given timout value looking any output on the STDIN,STDOUT,and
  //STDERR streams.
  //If the value of timeout is zero or greater we will wait that duration.
  //If the value of timeout is negative we will  block indefinitely until a
  //output on one of the pipes happens
  meshserver::common::ProcessPipe poll(double timeout);

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
