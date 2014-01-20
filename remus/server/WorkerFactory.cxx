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
#include <remus/common/ExecuteProcess.h>

#include <remus/common/MeshIOType.h>
#include <remus/common/MeshRegistrar.h>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace
{
  typedef std::vector<remus::server::MeshWorkerInfo>::const_iterator WorkerIterator;
  typedef std::vector< remus::server::WorkerFactory::RunningProcessInfo >::iterator ProcessIterator;


  //----------------------------------------------------------------------------
  struct support_MeshIOType
  {
    remus::common::MeshIOType Type;
    support_MeshIOType(remus::common::MeshIOType type):Type(type){}
    bool operator()(const remus::server::MeshWorkerInfo& info)
      {
      return info.Type == this->Type;
      }
  };

  //----------------------------------------------------------------------------
  struct is_dead
  {
    bool operator()(const remus::server::WorkerFactory::RunningProcessInfo& process) const
      {
      return !process.first->isAlive();
      }
  };

  //----------------------------------------------------------------------------
  struct kill_on_deletion
  {
    void operator()(remus::server::WorkerFactory::RunningProcessInfo& process) const
      {
      const bool can_be_killed =
        (process.second == remus::server::WorkerFactory::KillOnFactoryDeletion);
      const bool is_alive = !is_dead()(process);
      if(can_be_killed && is_alive)
        {
        process.first->kill();
        }
      }
  };

  //----------------------------------------------------------------------------
  struct ValidWorker
  {
    ValidWorker(std::string p):
      valid(true),
      path(p)
      {
      }

    ValidWorker():
      valid(false),
      path()
      {
      }

    bool valid;
    std::string path;
  };

  //----------------------------------------------------------------------------
  template<typename Container >
  ValidWorker find_worker_path(remus::common::MeshIOType type,
                               Container const& container)
  {
    support_MeshIOType pred(type);
    WorkerIterator result = std::find_if(container.begin(),container.end(),pred);

    if(result != container.end())
      {
      //return a valid work
      return ValidWorker((*result).ExecutionPath);
      }
    //return an invalid worker
    return ValidWorker();
  }

}


namespace remus{
namespace server{

//class that loads all files in the executing directory
//with the msw extension and creates a vector of possible
//meshers with that info
class WorkerFinder
{
public:
  typedef std::vector<MeshWorkerInfo>::const_iterator const_iterator;
  typedef std::vector<MeshWorkerInfo>::iterator iterator;
  const std::string FileExt;


  //----------------------------------------------------------------------------
  WorkerFinder(const std::string& ext):
    FileExt( boost::algorithm::to_upper_copy(ext) )
    {
    boost::filesystem::path cwd = boost::filesystem::current_path();
    this->parseDirectory(cwd);
    }

  //----------------------------------------------------------------------------
  WorkerFinder(const boost::filesystem::path& path,
               const std::string& ext):
    FileExt( boost::algorithm::to_upper_copy(ext) )
    {
    this->parseDirectory(path);
    }

  //----------------------------------------------------------------------------
  void parseDirectory(const boost::filesystem::path& dir)
    {
    boost::filesystem::directory_iterator end_itr;
    for( boost::filesystem::directory_iterator i( dir ); i != end_itr; ++i )
      {
      // Skip if not a file
      if(boost::filesystem::is_regular_file( i->status() ))
        {
        std::string ext = boost::algorithm::to_upper_copy(
                            i->path().extension().string());
        if(ext == FileExt)
          {
          this->parseFile(i->path());
          }
        }
      }
    }

  //----------------------------------------------------------------------------
  void parseFile(const boost::filesystem::path& file)
  {
    //open the file, parse two lines and close file
    boost::filesystem::ifstream f;
    f.open(file);
    if(f.is_open())
      {
      std::string inputFileType,outputMeshIOType,mesherName;
      getline(f,inputFileType);
      getline(f,outputMeshIOType);
      getline(f,mesherName);

      boost::filesystem::path mesher_path(mesherName);

      //try the mesherName as an absolute path, if that isn't
      //a file than fall back to looking based on the file we are parsing
      //path
      if(!boost::filesystem::is_regular_file(mesher_path))
        {
        mesher_path = boost::filesystem::path(file.parent_path());
        mesher_path /= mesherName;
#ifdef _WIN32
        mesher_path.replace_extension(".exe");
#endif
        }

      if(boost::filesystem::is_regular_file(mesher_path))
        {
        //convert the mesher_path into an absolute canonical path now
        mesher_path = boost::filesystem::canonical(mesher_path);
        remus::common::MeshIOType combinedType(
                               remus::meshtypes::to_meshType(inputFileType),
                               remus::meshtypes::to_meshType(outputMeshIOType)
                               );
        this->Info.push_back(MeshWorkerInfo(combinedType,
                                            mesher_path.string()));
        }
      }
    f.close();

  }

  const std::vector<MeshWorkerInfo>& results() const {return Info;}

  iterator begin() {return this->Info.begin();}
  const_iterator begin() const {return this->Info.begin();}

  iterator end() {return this->Info.end();}
  const_iterator end() const {return this->Info.end();}

private:
  std::vector<MeshWorkerInfo> Info;

};

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory():
  MaxWorkers(1),
  WorkerExtension(".RW"),
  PossibleWorkers(),
  CurrentProcesses(),
  GlobalArguments()
{
  WorkerFinder finder(this->WorkerExtension); //default to current working directory
  this->PossibleWorkers.insert(this->PossibleWorkers.end(),
                               finder.begin(),
                               finder.end());

}

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory(const std::string& ext):
  MaxWorkers(1),
  WorkerExtension(ext),
  PossibleWorkers(),
  CurrentProcesses(),
  GlobalArguments()
{
  WorkerFinder finder(this->WorkerExtension); //default to current working directory
  this->PossibleWorkers.insert(this->PossibleWorkers.end(),
                               finder.begin(),
                               finder.end());
}

//----------------------------------------------------------------------------
WorkerFactory::~WorkerFactory()
{
  //kill any worker whose FactoryDeletionBehavior is KillOnFactoryDeletion
  //the kill_on_deletion functor will call terminate
  std::for_each(this->CurrentProcesses.begin(),
                this->CurrentProcesses.end(),
                kill_on_deletion());
}

//----------------------------------------------------------------------------
void WorkerFactory::addCommandLineArgument(const std::string& argument)
{
  this->GlobalArguments.push_back(argument);
}

//----------------------------------------------------------------------------
void WorkerFactory::clearCommandLineArguments()
{
  this->GlobalArguments.clear();
}

//----------------------------------------------------------------------------
void WorkerFactory::addWorkerSearchDirectory(const std::string &directory)
{
  boost::filesystem::path dir(directory);
  WorkerFinder finder(dir,this->WorkerExtension);
  this->PossibleWorkers.insert(this->PossibleWorkers.end(),
                               finder.begin(),
                               finder.end());
}

//----------------------------------------------------------------------------
bool WorkerFactory::haveSupport(remus::common::MeshIOType type ) const
{
  return find_worker_path(type,this->PossibleWorkers).valid;
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(remus::common::MeshIOType type,
                               WorkerFactory::FactoryDeletionBehavior lifespan)
{
  this->updateWorkerCount(); //remove dead workers
  if(this->currentWorkerCount() < this->maxWorkerCount())
    {
    const ValidWorker w = find_worker_path(type, this->PossibleWorkers);
    if(w.valid)
      {
      return this->addWorker(w.path,lifespan);
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void WorkerFactory::updateWorkerCount()
{
  //foreach current worker remove any that return they are not alive
  this->CurrentProcesses.erase(remove_if(this->CurrentProcesses.begin(),
                                          this->CurrentProcesses.end(),
                                          is_dead()),
                               this->CurrentProcesses.end());
}

//----------------------------------------------------------------------------
bool WorkerFactory::addWorker(const std::string& executable,
                              WorkerFactory::FactoryDeletionBehavior lifespan)
{
  //add this workers
  WorkerFactory::ExecuteProcessPtr ep(
                        new ExecuteProcess(executable,this->GlobalArguments) );

  //we set the detached behavior based on if we want the worker to last
  //longer than us. We also need to store the lifespan flag, so that
  //we can hard terminate workers when we leave that have the
  //KillOnFactoryDeletion otherwise we hang while the continue to run
  if(lifespan == WorkerFactory::KillOnFactoryDeletion)
    {
    ep->execute( remus::common::ExecuteProcess::Detached );
    }
  else
    {
    ep->execute( remus::common::ExecuteProcess::Attached );
    }

  remus::server::WorkerFactory::RunningProcessInfo p_info(ep,lifespan);

  this->CurrentProcesses.push_back(p_info);
  return true;
}


}
}
