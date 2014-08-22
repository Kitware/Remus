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

#include <remus/common/SleepFor.h>
#include <remus/testing/Testing.h>
#include "PathToTestExecutable.h"

#include <iostream>

namespace
{
  const remus::common::ExecuteProcess::DetachMode detached =
      remus::common::ExecuteProcess::Detached;

  const remus::common::ExecuteProcess::DetachMode attached =
      remus::common::ExecuteProcess::Attached;
}

int UnitTestExecuteProcess(int, char *[])
{
  ExampleApplication eapp;

//==============================================================================
//  Test Launching and hard killing program
//==============================================================================

  //we need to launch example programs that we know will work on each os
  //for now that will be a terminal / shell
  {
  std::cout << eapp.name << std::endl;
  remus::common::ExecuteProcess example(eapp.name);

  //validate that isAlive and kill return the proper results before
  //we call execute
  REMUS_ASSERT(!example.isAlive());
  REMUS_ASSERT(!example.kill());

  //start the process in non-detached mode
  example.execute(attached);

  //validate the program is running, and than terminate it
  REMUS_ASSERT(example.isAlive());
  REMUS_ASSERT(example.kill()); //kill

  //make sure the program terminated
  REMUS_ASSERT(!example.isAlive());
  REMUS_ASSERT(!example.exitedNormally());
  REMUS_ASSERT(!example.kill()); //can't kill twice

  //start the process in detached mode
  example.execute(detached);

  //validate the program is running, and than terminate it
  REMUS_ASSERT(example.isAlive());
  REMUS_ASSERT(example.kill()); //kill

  //make sure the program terminated
  REMUS_ASSERT(!example.isAlive());
  REMUS_ASSERT(!example.exitedNormally());
  REMUS_ASSERT(!example.kill()); //can't kill twice
  }


//==============================================================================
//  Test Launching a program that exits it self
//==============================================================================
  {
  //next create a program that will exit normally, and we won't have to kill
  std::vector< std::string > args;
  args.push_back("SLEEP_AND_EXIT");
  remus::common::ExecuteProcess example(eapp.name, args );

  //start the process in non detached-mode
  example.execute(attached);
  REMUS_ASSERT(example.isAlive());

  //wait for the example program to close itself
  while(example.isAlive()){}

  REMUS_ASSERT(example.exitedNormally());
  //make sure we can't kill a program that has stopped
  REMUS_ASSERT(!example.kill());

  //start the process in detached-mode
  example.execute(detached);
  REMUS_ASSERT(example.isAlive());

  //wait for the example program to close itself
  while(example.isAlive()){}

  REMUS_ASSERT(example.exitedNormally());
  //make sure we can't kill a program that has stopped
  REMUS_ASSERT(!example.kill());
  }


//==============================================================================
//  Test Polling on standard out channel
//==============================================================================
  {
  //next step will be to test that we can properly poll an application
  std::vector< std::string > args;
  args.push_back("COUT_OUTPUT");
  remus::common::ExecuteProcess example(eapp.name, args );

  //start the process in attached mode, and test polling
  example.execute(attached);
  REMUS_ASSERT(example.isAlive());

  //verify the polling results
  remus::common::ProcessPipe pollResult = example.poll(-1);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDOUT) );

  pollResult = example.poll(10);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDOUT) );

  pollResult = example.poll(1);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDOUT) );

  remus::common::SleepForMillisec(1000);
  pollResult = example.poll(0);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDOUT) );

  //kill the process, and make sure it is terminated
  REMUS_ASSERT(example.kill());
  }

//==============================================================================
//  Test Polling on standard error channel
//==============================================================================
  {
  //next step will be to test that we can properly poll an application
  std::vector< std::string > args;
  args.push_back("CERR_OUTPUT");
  remus::common::ExecuteProcess example(eapp.name, args );

  //start the process in attached mode, and test polling
  example.execute(attached);
  REMUS_ASSERT(example.isAlive());

  //verify the polling results
  remus::common::ProcessPipe pollResult = example.poll(-1);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDERR) );

  pollResult = example.poll(10);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDERR) );

  pollResult = example.poll(1);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDERR) );

  remus::common::SleepForMillisec(1000);
  pollResult = example.poll(0);
  REMUS_ASSERT(pollResult.valid());
  REMUS_ASSERT( (pollResult.type == remus::common::ProcessPipe::STDERR) );

  //kill the process, and make sure it is terminated
  REMUS_ASSERT(example.kill());
  }

//==============================================================================
//  Test Polling timeout
//==============================================================================

  {
  //next create a program that will not have output so the polling will
  //timeout
  std::vector< std::string > args;
  args.push_back("NO_OUTPUT");
  remus::common::ExecuteProcess example(eapp.name, args );
  remus::common::ProcessPipe noPoll = example.poll(0.1);
  REMUS_ASSERT(!noPoll.valid());

  //start the process in detached mode
  example.execute(detached);
  REMUS_ASSERT(example.isAlive());

  //try polling for different lengths of time
  //all should fail
  //The timeout's unit of time is SECONDS.
  noPoll = example.poll(0.5);
  REMUS_ASSERT(!noPoll.valid());

  noPoll = example.poll(0.001);
  REMUS_ASSERT(!noPoll.valid());

  noPoll = example.poll(0);
  REMUS_ASSERT(!noPoll.valid());

  //kill the process, and make sure it is terminated
  REMUS_ASSERT(example.kill());

  //start the process in non-detached mode
  example.execute(attached);
  REMUS_ASSERT(example.isAlive());

  //try polling for different lengths of time
  //all should fail
  //The timeout's unit of time is SECONDS.
  noPoll = example.poll(0.5);
  REMUS_ASSERT(!noPoll.valid());

  noPoll = example.poll(0.001);
  REMUS_ASSERT(!noPoll.valid());

  noPoll = example.poll(0);
  REMUS_ASSERT(!noPoll.valid());

  //kill the process, and make sure it is terminated
  REMUS_ASSERT(example.kill());
  }


  return 0;
}
