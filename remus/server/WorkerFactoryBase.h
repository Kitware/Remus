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

#ifndef remus_server_WorkeryFactoryBase_h
#define remus_server_WorkeryFactoryBase_h

#include <vector>

#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <remus/common/MeshIOType.h>
#include <remus/proto/JobRequirements.h>

//included for export symbols
#include <remus/server/ServerExports.h>

#include <remus/common/CompilerInformation.h>
#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif


namespace remus{
namespace server{

//forward declare the port connection class
class PortConnection;


//The Worker Factory Base task.
//A common interface for abstracting out how the server can ask for workers
//to be launched.
//You can control the number of launched workers, by calling setMaxWorkerCount.
//When is a worker is launched you can control if you want that worker terminated
//when the WorkerFactory instance gets deleted. If you created workers that stay after
//the factory is deleted, you better make sure they are connected to the server,
//or you will have zombie workers
class REMUSSERVER_EXPORT WorkerFactoryBase
{
public:
  enum FactoryDeletionBehavior
  {
    LiveOnFactoryDeletion,
    KillOnFactoryDeletion
  };

  //by default sets the number of max workers to 1.
  WorkerFactoryBase();

  virtual ~WorkerFactoryBase();

  //add command line argument to be passed to all workers that
  //are created.
  void addCommandLineArgument(const std::string& argument)
    { this->GlobalCommandLineArguments.push_back(argument); }

  const std::vector< std::string >& commandLineArguments() const
    { return this->GlobalCommandLineArguments; }

  //remove all command line arguments to be passed to all workers
  //that the factory creates
  void clearCommandLineArguments();

  //specify the port/endpoint that workers need to use to connect to the
  //running server
  void portForWorkersToUse(const remus::server::PortConnection& port);

  const std::string& workerEndpoint() const
    { return this->WorkerEndpoint; }

  //return all the MeshIOTypes that the factory can possibly make
  //this allows the client to discover workers types that can be constructed
  virtual remus::common::MeshIOTypeSet supportedIOTypes() const =0;

  virtual remus::proto::JobRequirementsSet workerRequirements(
                                       remus::common::MeshIOType type) const =0;

  virtual bool haveSupport(const remus::proto::JobRequirements& reqs) const =0;

  virtual bool createWorker(const remus::proto::JobRequirements& type,
                            WorkerFactoryBase::FactoryDeletionBehavior lifespan) = 0;

  virtual void updateWorkerCount() = 0;

  //Set the maximum number of total workers that can be returning at once
  void setMaxWorkerCount(unsigned int count){MaxWorkers = count;}
  unsigned int maxWorkerCount() const {return MaxWorkers;}
  virtual unsigned int currentWorkerCount() const =0;

private:
  unsigned int MaxWorkers;
  std::string WorkerEndpoint;

  std::vector<std::string> GlobalCommandLineArguments;
};

}
}

#ifdef REMUS_MSVC
#pragma warning(pop)
#endif

#endif
