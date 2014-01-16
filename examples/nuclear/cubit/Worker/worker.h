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

#ifndef _Cubit_Worker_worker_h
#define _Cubit_Worker_worker_h

#include <remus/worker/Job.h>
#include <remus/worker/ServerConnection.h>
#include <remus/worker/Worker.h>

#include <remus/common/ExecuteProcess.h>

#include "CubitInput.h"

class worker : public remus::worker::Worker
{
public:
  worker(remus::worker::ServerConnection const& connection);
  ~worker();
  
  //will get a triangle job from the remus server
  //and will call triangle inside a its own thread to mesh the job
  void meshJob();
  
protected:
  void launchProcess(const CubitInput& job);
  void cleanlyExit();
  void jobFailed(const remus::worker::Job& job);
  bool pollStatus(const remus::worker::Job& job);
  remus::common::ExecuteProcess* Process;
};

#endif
