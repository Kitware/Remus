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

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace
{
  typedef std::vector<remus::server::MeshWorkerInfo>::const_iterator WorkerIterator;
  typedef std::vector< boost::shared_ptr<remus::common::ExecuteProcess> >::iterator ProcessIterator;

  struct support_MeshIOType
  {
    remus::common::MeshIOType Type;
    support_MeshIOType(remus::common::MeshIOType type):Type(type){}
    bool operator()(const remus::server::MeshWorkerInfo& info)
      {
      return info.Type == this->Type;
      }
  };

  struct is_dead
  {
    bool operator()(boost::shared_ptr<remus::common::ExecuteProcess> ep)
      {
      return !ep->isAlive();
      }
  };
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


  WorkerFinder(const std::string& ext):
    FileExt(ext)
    {
    boost::filesystem::path cwd = boost::filesystem::current_path();
    this->parseDirectory(cwd);
    }

  WorkerFinder(const boost::filesystem::path& path,
               const std::string& ext):
    FileExt(ext)
    {
    this->parseDirectory(path);
    }

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

      //convert from string to the proper types
      remus::MESH_INPUT_TYPE itype = remus::to_meshInType(inputFileType);
      remus::MESH_OUTPUT_TYPE otype = remus::to_meshOutType(outputMeshIOType);
      boost::filesystem::path p(file.parent_path());
      p /= mesherName;
#ifdef WIN32
      p.replace_extension(".exe");
#endif
      if(boost::filesystem::is_regular_file(p))
        {
        remus::common::MeshIOType combinedType(itype,otype);
        this->Info.push_back(MeshWorkerInfo(combinedType, p.string()));
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
}

//----------------------------------------------------------------------------
void WorkerFactory::addCommandLineArgument(const std::string& argument)
{
  this->GlobalArguments.push_back(argument);
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
  support_MeshIOType pred(type);
  WorkerIterator result = std::find_if(this->PossibleWorkers.begin(),
                      this->PossibleWorkers.end(),
                      pred);
  return result != this->PossibleWorkers.end();
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(remus::common::MeshIOType type)
{
  this->updateWorkerCount();
  if(this->currentWorkerCount() >= this->maxWorkerCount())
    {
    return false;
    }

  support_MeshIOType pred(type);
  WorkerIterator result = std::find_if(this->PossibleWorkers.begin(),
                      this->PossibleWorkers.end(),
                      pred);

  if(result == this->PossibleWorkers.end())
    {
    return false;
    }
  return this->addWorker( (*result).ExecutionPath );
}

//----------------------------------------------------------------------------
void WorkerFactory::updateWorkerCount()
{
  //foreach current worker remove any that return they are not alive
  ProcessIterator result = std::remove_if(this->CurrentProcesses.begin(),
                                          this->CurrentProcesses.end(),
                                          is_dead());
  std::size_t numDead = std::distance(result,this->CurrentProcesses.end());
  // if( numDead > 0)
  //   {
  //   std::cout << "Removing dead workers processes, num " << numDead << std::endl;
  //   }
  this->CurrentProcesses.resize(this->CurrentProcesses.size()-numDead);
}


//----------------------------------------------------------------------------
bool WorkerFactory::addWorker(const std::string& executable)
{
  //add this workers
  ExecuteProcessPtr ep(new ExecuteProcess(executable,this->GlobalArguments) );
  ep->execute(true); //detach worker
  this->CurrentProcesses.push_back(ep);
  return true;
}


}
}
