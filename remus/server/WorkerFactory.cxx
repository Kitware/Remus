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

#include <remus/server/WorkerFactory.h>

#include <remus/common/CompilerInformation.h>
#include <remus/common/ExecuteProcess.h>
#include <remus/common/MeshIOType.h>
#include <remus/server/FactoryFileParser.h>
#include <remus/server/detail/WorkerFinder.h>

//force to use filesystem version 3
REMUS_THIRDPARTY_PRE_INCLUDE
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <algorithm>

namespace
{
  //typedefs required
  typedef remus::common::ExecuteProcess ExecuteProcess;
  typedef boost::shared_ptr<ExecuteProcess> ExecuteProcessPtr;
  typedef std::pair<ExecuteProcessPtr,
                    remus::server::WorkerFactoryBase::FactoryDeletionBehavior>
              RunningProcessInfo;

  typedef std::vector<remus::server::FactoryWorkerSpecification>::const_iterator WorkerIterator;
  typedef std::vector< RunningProcessInfo >::iterator ProcessIterator;

  //----------------------------------------------------------------------------
  struct support_MeshIOType
  {
    remus::common::MeshIOType Type;
    support_MeshIOType(remus::common::MeshIOType type):Type(type)
      {
      }

    bool operator()(const remus::server::FactoryWorkerSpecification& info)
      {
      return info.Requirements.meshTypes() == this->Type;
      }
  };

  //----------------------------------------------------------------------------
  struct support_JobReqs
  {
    remus::proto::JobRequirements& Requirements;
    support_JobReqs(remus::proto::JobRequirements& reqs):
      Requirements(reqs)
      {
      }

    bool operator()(const remus::server::FactoryWorkerSpecification& info)
      {
      return info.Requirements == this->Requirements;
      }
  };

  //----------------------------------------------------------------------------
  struct is_dead
  {
    bool operator()(const RunningProcessInfo& process) const
      {
      return !process.first->isAlive();
      }
  };

  //----------------------------------------------------------------------------
  struct kill_on_deletion
  {
    void operator()(RunningProcessInfo& process) const
      {
      is_dead isDead;
      const bool shouldBeTerminated =
        (process.second == remus::server::WorkerFactoryBase::KillOnFactoryDeletion);
      const bool is_alive = !isDead(process);
      if(shouldBeTerminated && is_alive)
        {
        process.first->kill();
        }
      }
  };

  //----------------------------------------------------------------------------
  struct ValidWorker
  {
    ValidWorker(const remus::server::FactoryWorkerSpecification& w):
      valid(true),
      spec(w)
      {
      }

    ValidWorker():
      valid(false)
      {
      }

    bool valid;
    remus::server::FactoryWorkerSpecification spec;
  };


  //----------------------------------------------------------------------------
  template<typename Container >
  ValidWorker find_worker_path(remus::proto::JobRequirements reqs,
                               Container const& container)
  {
    support_JobReqs pred(reqs);
    WorkerIterator result = std::find_if(container.begin(),container.end(),pred);

    if(result != container.end())
      {
      //return a valid work
      return ValidWorker(*result);
      }
    //return an invalid worker
    return ValidWorker();
  }

    //----------------------------------------------------------------------------
  template<typename Container >
  remus::common::MeshIOTypeSet
  find_all_workers_iotypes(Container const& container)
  {
    typedef typename Container::const_iterator IterType;
    remus::common::MeshIOTypeSet validIOTypes;
    for(IterType i = container.begin(); i != container.end(); ++i)
      { validIOTypes.insert(i->Requirements.meshTypes()); }
    return validIOTypes;
  }

  //----------------------------------------------------------------------------
  template<typename Container >
  remus::proto::JobRequirementsSet
  find_all_matching_workers(remus::common::MeshIOType type,
                            Container const& container)
  {
    typedef typename Container::const_iterator IterType;
    support_MeshIOType pred(type);
    remus::proto::JobRequirementsSet validWorkers;
    for(IterType i = container.begin(); i != container.end(); ++i)
      {
      if(pred(*i)) { validWorkers.insert(i->Requirements); }
      }
    return validWorkers;
  }
}


namespace remus{
namespace server{


struct WorkerFactory::WorkerTracker
{
  WorkerTracker():
    PossibleWorkers(),
    CurrentProcesses()
    {

    }

  bool addWorker(
    const remus::server::FactoryWorkerSpecification& workerSpec,
    WorkerFactoryBase::FactoryDeletionBehavior lifespan);

  std::vector< remus::server::FactoryWorkerSpecification > PossibleWorkers;
  std::vector< RunningProcessInfo > CurrentProcesses;

};

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory():
  WorkerFactoryBase(),
  WorkerExtension(".RW"),
  Parser( boost::make_shared<FactoryFileParser>() ),
  Tracker(boost::make_shared<WorkerTracker>())
{
  //default to current working directory
  remus::server::detail::WorkerFinder finder(this->Parser,
                                             this->WorkerExtension);
  this->Tracker->PossibleWorkers.insert(this->Tracker->PossibleWorkers.end(),
                                          finder.begin(),
                                          finder.end());

}

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory(const std::string& ext):
  WorkerFactoryBase(),
  WorkerExtension(ext),
  Parser( boost::make_shared<FactoryFileParser>() ),
  Tracker(boost::make_shared<WorkerTracker>())
{
  //default to current working directory
  remus::server::detail::WorkerFinder finder(this->Parser,
                                             this->WorkerExtension);
  this->Tracker->PossibleWorkers.insert(this->Tracker->PossibleWorkers.end(),
                                        finder.begin(),
                                        finder.end());
}

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory(const std::string& ext,
                             boost::shared_ptr<FactoryFileParser>& parser):
  WorkerFactoryBase(),
  WorkerExtension(ext),
  Parser( parser ),
  Tracker(boost::make_shared<WorkerTracker>())
{
  //default to current working directory
  remus::server::detail::WorkerFinder finder(this->Parser,
                                             this->WorkerExtension);
  this->Tracker->PossibleWorkers.insert(this->Tracker->PossibleWorkers.end(),
                                        finder.begin(),
                                        finder.end());
}

//----------------------------------------------------------------------------
WorkerFactory::~WorkerFactory()
{
  //kill any worker whose FactoryDeletionBehavior is KillOnFactoryDeletion
  //the kill_on_deletion functor will call terminate
  std::for_each(this->Tracker->CurrentProcesses.begin(),
                this->Tracker->CurrentProcesses.end(),
                kill_on_deletion());
}

//----------------------------------------------------------------------------
void WorkerFactory::addWorkerSearchDirectory(const std::string &directory)
{
  boost::filesystem::path dir(directory);
  remus::server::detail::WorkerFinder finder(this->Parser,
                                             dir,
                                             this->WorkerExtension);
  this->Tracker->PossibleWorkers.insert(this->Tracker->PossibleWorkers.end(),
                                        finder.begin(),
                                        finder.end());
}

//----------------------------------------------------------------------------
remus::common::MeshIOTypeSet WorkerFactory::supportedIOTypes() const
{
  return find_all_workers_iotypes(this->Tracker->PossibleWorkers);
}

//----------------------------------------------------------------------------
remus::proto::JobRequirementsSet WorkerFactory::workerRequirements(
                                       remus::common::MeshIOType type) const
{
  return find_all_matching_workers(type,this->Tracker->PossibleWorkers);
}

//----------------------------------------------------------------------------
bool WorkerFactory::haveSupport(
                            const remus::proto::JobRequirements& reqs) const
{
  return find_worker_path(reqs, this->Tracker->PossibleWorkers).valid;
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(const remus::proto::JobRequirements& reqs,
                                 WorkerFactoryBase::FactoryDeletionBehavior lifespan)
{
  //we check if we can create a worker with a given set of requirements, before
  //we check for space, since the 'updateWorkerCount' call is really really
  //slow ( milliseconds ), while the 'ValidWorker' check is
  //fast ( microseconds ).
  //This is super important as 'Server::FindWorkerForQueuedJob' can really hammer
  //this method
  const ValidWorker w = find_worker_path(reqs, this->Tracker->PossibleWorkers);
  if(w.valid)
    {
    this->updateWorkerCount(); //remove dead workers
    if(this->currentWorkerCount() < this->maxWorkerCount())
      {
      return this->addWorker(w.spec, lifespan);
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void WorkerFactory::updateWorkerCount()
{
  //foreach current worker remove any that return they are not alive
  this->Tracker->CurrentProcesses.erase(
      std::remove_if(this->Tracker->CurrentProcesses.begin(),
                     this->Tracker->CurrentProcesses.end(),
                     is_dead()),
      this->Tracker->CurrentProcesses.end());
}

//----------------------------------------------------------------------------
unsigned int WorkerFactory::currentWorkerCount() const
{
  return static_cast<unsigned int>(this->Tracker->CurrentProcesses.size());
}

//----------------------------------------------------------------------------
bool WorkerFactory::addWorker(
  const FactoryWorkerSpecification& spec,
  WorkerFactoryBase::FactoryDeletionBehavior lifespan)
{
  //we currently can't support detached processes.
  if(lifespan == WorkerFactoryBase::LiveOnFactoryDeletion)
    {
    return false;
    }

  //add this workers
  std::vector< std::string > arguments;
  if ( !this->workerEndpoint().empty() )
    { //only add the end point if it is not empty
    arguments.push_back( this->workerEndpoint() );
    }
  const std::vector< std::string >& cmlArgs = this->commandLineArguments();
  arguments.insert( arguments.end(), cmlArgs.begin(), cmlArgs.end() );
  arguments.insert( arguments.end(), spec.ExtraCommandLineArguments.begin(), spec.ExtraCommandLineArguments.end() );

  ExecuteProcessPtr ep(
    boost::make_shared<ExecuteProcess>(
      spec.ExecutionPath.string(), arguments, spec.EnvironmentVariables
      )
    );

  //launch all process in attached mode, that way we can determine if
  //they are still alive or not. Once a process goes to detached mode
  //it is impossible to determine if it is still running or not
  ep->execute( );

  RunningProcessInfo p_info(ep,lifespan);

  this->Tracker->CurrentProcesses.push_back(p_info);
  return true;
}


}
}
