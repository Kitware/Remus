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

#ifndef remus_server_WorkeryFactory_h
#define remus_server_WorkeryFactory_h

#include <vector>

#include <remus/common/MeshIOType.h>
#include <remus/proto/JobRequirements.h>

#include <boost/shared_ptr.hpp>

//included for export symbols
#include <remus/server/ServerExports.h>

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
  remus::proto::JobRequirements Requirements;
  std::string ExecutionPath;
  MeshWorkerInfo(const remus::proto::JobRequirements& r,
                 const std::string& p):
    Requirements(r),ExecutionPath(p){}
};

//The Worker Factory has two tasks.
//First it locates all files that match a given extension of the default extension
//of .rw. These files are than parsed to determine what type of local Remus workers
//we can launch.
//The second tasks the factory has is the ability to launch workers when requested.
//You can control the number of launched workers, by calling setMaxWorkerCount.
//When is a worker is launched you can control if you want that worker terminated
//when the WorkerFactory gets deleted. If you created workers that stay after
//the factory is deleted, you better make sure they are connected to the server,
//or you will have zombie processes
class REMUSSERVER_EXPORT WorkerFactory
{
public:
  enum FactoryDeletionBehavior
  {
    LiveOnFactoryDeletion,
    KillOnFactoryDeletion
  };

  //Work Factory defaults to creating a maximum of 1 workers at once,
  //with a default extension to search for of "rw"
  WorkerFactory();

  //Create a worker factory with a different file extension to
  //search for. The extension must start with a period.
  WorkerFactory(const std::string& ext);

  virtual ~WorkerFactory();

  //add command line argument to be passed to all workers that
  //are created
  void addCommandLineArgument(const std::string& argument);

  //remove all command line arguments to be passed to all workers
  //that the factory creates
  void clearCommandLineArguments();

  //add a path to search for workers.
  //by default we only search the current working directory
  void addWorkerSearchDirectory(const std::string& directory);

  //return all the MeshIOTypes that the factory can possibly make
  //this allows the client to discover workers types that can be constructed
  virtual remus::common::MeshIOTypeSet supportedIOTypes() const;

  //return all the JobRequirementsSet for all workers that match a give
  //MeshIOType
  virtual remus::proto::JobRequirementsSet workerRequirements(
                                       remus::common::MeshIOType type) const;

  //return if the worker factory has a worker that matches the given requirements
  virtual bool haveSupport(const remus::proto::JobRequirements& reqs) const;

  //request the factory to construct a worker given a requirements and a lifespan
  //can return false if the factory doesn't support these requirements, or
  //if the factory already has too many workers in existence already.
  virtual bool createWorker(const remus::proto::JobRequirements& type,
                            WorkerFactory::FactoryDeletionBehavior lifespan);

  //checks all current processes and removes any that have
  //shutdown
  virtual void updateWorkerCount();

  //Set the maximum number of total workers that can be returning at once
  void setMaxWorkerCount(unsigned int count){MaxWorkers = count;}
  unsigned int maxWorkerCount(){return MaxWorkers;}
  unsigned int currentWorkerCount() const { return this->CurrentProcesses.size(); }

  //return the worker file extension we have
  std::string workerExtension() const { return this->WorkerExtension;  }


  //typedefs required
  typedef remus::common::ExecuteProcess ExecuteProcess;
  typedef boost::shared_ptr<ExecuteProcess> ExecuteProcessPtr;
  typedef std::pair<ExecuteProcessPtr, WorkerFactory::FactoryDeletionBehavior>
            RunningProcessInfo;

private:
  //this method only handles constructing the worker
  //it is expected that all checks to make sure that the worker type
  //have been done, and that we have room for the worker already
  virtual bool addWorker(const std::string& executable,
                         FactoryDeletionBehavior lifespan);

  unsigned int MaxWorkers;
  std::string WorkerExtension;

  std::vector<MeshWorkerInfo> PossibleWorkers;
  std::vector< RunningProcessInfo > CurrentProcesses;
  std::vector<std::string> GlobalArguments;

};

}
}

#endif
