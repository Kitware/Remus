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

#ifndef __remus_server_WorkeryFactory_h
#define __remus_server_WorkeryFactory_h

#include <vector>
#include <remus/common/remusGlobals.h>
#include <boost/shared_ptr.hpp>

//included for symbol exports
#include "ServerExports.h"

//forward declare the execute process
namespace remus{
namespace common{
class ExecuteProcess;
}
}

namespace remus{
namespace server{

struct REMUSSERVER_EXPORT MeshWorkerInfo
{
  remus::MESH_TYPE Type;
  std::string ExecutionPath;
  MeshWorkerInfo(remus::MESH_TYPE t, const std::string& p):
    Type(t),ExecutionPath(p){}
};

class REMUSSERVER_EXPORT WorkerFactory
{
public:
  //Work Factory defaults to creating a maximum of 1 workers at once
  WorkerFactory();
  virtual ~WorkerFactory();

  //add command line argument to be passed to all workers that
  //are created
  void addCommandLineArgument(const std::string& argument);

  //add a path to search for workers.
  //by default we only search the current working directory
  void addWorkerSearchDirectory(const std::string& directory);

  virtual bool haveSupport(remus::MESH_TYPE type ) const;
  virtual bool createWorker(remus::MESH_TYPE type);

  //checks all current processes and removes any that have
  //shutdown
  virtual void updateWorkerCount();

  //Set the maximum number of total workers that can be returning at once
  void setMaxWorkerCount(unsigned int count){MaxWorkers = count;}
  unsigned int maxWorkerCount(){return MaxWorkers;}
  unsigned int currentWorkerCount() const { return this->CurrentProcesses.size(); }


protected:
  typedef remus::common::ExecuteProcess ExecuteProcess;
  typedef boost::shared_ptr<ExecuteProcess> ExecuteProcessPtr;

  virtual bool addWorker(const std::string& executable);

  unsigned int MaxWorkers;
  std::vector<MeshWorkerInfo> PossibleWorkers;
  std::vector<ExecuteProcessPtr> CurrentProcesses;
  std::vector<std::string> GlobalArguments;
};

}
}

#endif
