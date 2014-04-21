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

#include <algorithm>

//include cjson for parsing the mesh worker file
#include "cJSON.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <iostream>

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
      return info.Requirements.meshTypes() == this->Type;
      }
  };

  //----------------------------------------------------------------------------
  struct support_JobReqs
  {
    remus::proto::JobRequirements& Requirements;
    support_JobReqs(remus::proto::JobRequirements& reqs):
      Requirements(reqs)
    {}
    bool operator()(const remus::server::MeshWorkerInfo& info)
      {
      return info.Requirements == this->Requirements;
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
  ValidWorker find_worker_path(remus::proto::JobRequirements reqs,
                               Container const& container)
  {
    support_JobReqs pred(reqs);
    WorkerIterator result = std::find_if(container.begin(),container.end(),pred);

    if(result != container.end())
      {
      //return a valid work
      return ValidWorker((*result).ExecutionPath);
      }
    //return an invalid worker
    return ValidWorker();
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

  //----------------------------------------------------------------------------
  std::string read_file(const boost::filesystem::path& file)
  {
    std::string file_contents;
    std::string path = boost::filesystem::canonical(file).string();
    std::ifstream f(path.c_str(),
                    std::ios_base::in | std::ios::binary);
    if(f.is_open())
      {
      //read the file into a std::string
      f.seekg(0, std::ios::end);
      file_contents.resize( f.tellg() );
      f.seekg(0, std::ios::beg);
      f.read(&file_contents[0], file_contents.size());
      }
    f.close();
    return file_contents;
  }

  //----------------------------------------------------------------------------
  std::pair< boost::filesystem::path, remus::proto::JobRequirements >
  parse_json_reqs( const boost::filesystem::path& file )
  {
    std::string json_contents = read_file(file);
    using namespace remus::common;

    cJSON *root = cJSON_Parse(json_contents.c_str());

    cJSON *inputT = cJSON_GetObjectItem(root,"InputType");
    cJSON *outputT = cJSON_GetObjectItem(root,"OutputType");
    cJSON *execT = cJSON_GetObjectItem(root,"ExecutableName");
    if(!inputT || !outputT || !execT)
      {
      return std::make_pair( boost::filesystem::path(),
                             remus::proto::JobRequirements() );
      }
    std::string inputType(inputT->valuestring);
    std::string outputType(outputT->valuestring);
    std::string executableName(execT->valuestring);

    //by default we select memory and user
    ContentFormat::Type format_type = ContentFormat::User;
    MeshIOType mesh_type( remus::meshtypes::to_meshType(inputType),
                          remus::meshtypes::to_meshType(outputType)
                          );

    //executableName can be a path, so figure it out
    boost::filesystem::path mesher_path(executableName);
    if(boost::filesystem::is_regular_file(mesher_path))
      {
      executableName = mesher_path.filename().string();
      }


    //make the basic memory type reqs
    remus::proto::JobRequirements reqs(format_type,
                                       mesh_type,
                                       executableName,
                                       std::string());

    //check if we have a specific file requirements
    cJSON *req_file = cJSON_GetObjectItem(root,"File");
    cJSON *req_file_format = cJSON_GetObjectItem(root,"FileFormat");
    if(req_file)
      {
      //we have a file source type, now determine the format type, and
      //read in the data from the file
      if(req_file_format)
        {
        if(strncmp(req_file_format->valuestring,"XML",3) == 0)
          { format_type = ContentFormat::XML; }
        else if(strncmp(req_file_format->valuestring,"JSON",4) == 0)
          { format_type = ContentFormat::JSON; }
        else if(strncmp(req_file_format->valuestring,"BSON",4) == 0)
          { format_type = ContentFormat::BSON; }
        }

      //now try to read the file
      boost::filesystem::path req_file_path(req_file->valuestring);
      if(!boost::filesystem::is_regular_file(req_file_path))
        {
        req_file_path = boost::filesystem::path(file.parent_path());
        req_file_path /= req_file->valuestring;
        }

      if(boost::filesystem::is_regular_file(req_file_path))
        {
        //load the contents of the file into a string to be given to the
        //requirements
        remus::common::FileHandle rfile(
                        boost::filesystem::canonical(req_file_path).string());
        reqs = remus::proto::JobRequirements(format_type,
                                             mesh_type,
                                             executableName,
                                             rfile);

        }
      }
    cJSON_Delete(root);

    return std::make_pair( mesher_path, reqs );
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
    if( boost::filesystem::is_directory(dir) )
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
    }

  //----------------------------------------------------------------------------
  void parseFile(const boost::filesystem::path& file )
  {
    typedef std::pair< boost::filesystem::path,
                       remus::proto::JobRequirements > ReturnType;
    ReturnType info = parse_json_reqs( file );

    //try the executableName as an absolute path, if that isn't
    //a file than fall back to looking based on the file we are parsing
    //path
    boost::filesystem::path mesher_path(info.first);
    if(!boost::filesystem::is_regular_file(mesher_path))
      {
      boost::filesystem::path new_path(file.parent_path());
      new_path /= info.first;
#ifdef _WIN32
      new_path.replace_extension(".exe");
#endif
      mesher_path = new_path;
      }

    if(boost::filesystem::is_regular_file(mesher_path))
      {
      //convert the mesher_path into an absolute canonical path now
      mesher_path = boost::filesystem::canonical(mesher_path);
      this->Info.push_back(MeshWorkerInfo(info.second,mesher_path.string()));
      }
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
remus::proto::JobRequirementsSet WorkerFactory::workerRequirements(
                                       remus::common::MeshIOType type) const
{
  return find_all_matching_workers(type,this->PossibleWorkers);
}

//----------------------------------------------------------------------------
bool WorkerFactory::haveSupport(
                            const remus::proto::JobRequirements& reqs) const
{
  return find_worker_path(reqs, this->PossibleWorkers).valid;
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(const remus::proto::JobRequirements& reqs,
                               WorkerFactory::FactoryDeletionBehavior lifespan)
{
  this->updateWorkerCount(); //remove dead workers
  if(this->currentWorkerCount() < this->maxWorkerCount())
    {
    const ValidWorker w = find_worker_path(reqs, this->PossibleWorkers);
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
