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

#ifndef remus_server_ThreadWorkerFactory_h
#define remus_server_ThreadWorkerFactory_h

#include <functional>
#include <map>
#include <memory>

//included for export symbols
#include <remus/server/ServerExports.h>
#include <remus/server/WorkerFactoryBase.h>

#include <remus/common/CompilerInformation.h>
#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif

namespace remus{
namespace proto{
class JobRequirements;
}
}

namespace remus{
namespace server{

//The Thread Worker Factory.
class REMUSSERVER_EXPORT ThreadWorkerFactory : public WorkerFactoryBase
{
  typedef proto::JobRequirements JobRequirements;

public:
  typedef std::function<void(const remus::proto::JobRequirements&,
                             const std::string&)> WorkerThread;

  //Thread Work Factory defaults to creating a maximum of 1 workers at once.
  ThreadWorkerFactory();
  virtual ~ThreadWorkerFactory();

  //return all the MeshIOTypes that the factory can possibly make
  //this allows the client to discover worker types that can be constructed
  virtual remus::common::MeshIOTypeSet supportedIOTypes() const;

  //return all the JobRequirementsSet for all workers that match a give
  //MeshIOType
  virtual remus::proto::JobRequirementsSet workerRequirements(
    remus::common::MeshIOType type) const;

  //return if the worker factory has a worker that matches the given requirements
  virtual bool haveSupport(const remus::proto::JobRequirements& reqs) const;

  //register a WorkerThread that this factory can launch.
  bool registerWorkerType(const remus::proto::JobRequirements& requirements,
                          WorkerThread worker);

  //request the factory to launch a worker given the job's requirements. If the
  //lifespan is set to <LiveOnFactoryDeletion>, if the factory doesn't support these
  //requirements, or if the factory already has too many workers in existence, the
  //job is not scheduled and the function returns false.
  virtual bool createWorker(const remus::proto::JobRequirements& type,
                            WorkerFactoryBase::FactoryDeletionBehavior lifespan);

  //request the factory to construct a worker given the job's requirements. If the
  //factory doesn't support these requirements, or if the factory already has too
  //many workers in existence, the job is not scheduled and the function returns
  //false.
  bool launchWorkerThread( const remus::proto::JobRequirements& requirements);

  virtual void setMaxWorkerCount(unsigned int count);
  virtual unsigned int maxWorkerCount() const;
  virtual unsigned int currentWorkerCount() const;
  virtual void updateWorkerCount() {}

private:
  //this method only handles constructing the worker
  //it is expected that all checks to make sure that the worker type
  //have been done, and that we have room for the worker already
  virtual bool addWorker(std::function<void()> workerWithTask);

  std::map<JobRequirements, WorkerThread> WorkerThreadTypes;

  class ThreadPool;
  ThreadPool* Pool;
};

}
}

#endif
