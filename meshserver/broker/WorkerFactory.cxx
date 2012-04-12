/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/WorkerFactory.h>
#include <meshserver/common/ExecuteProcess.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace
{
  typedef std::vector<meshserver::broker::MeshWorkerInfo>::const_iterator WorkerIterator;
  typedef std::vector< boost::shared_ptr<meshserver::common::ExecuteProcess> >::iterator ProcessIterator;

  struct support_meshType
  {
    meshserver::MESH_TYPE Type;
    support_meshType(meshserver::MESH_TYPE type):Type(type){}
    bool operator()(const meshserver::broker::MeshWorkerInfo& info)
      {
      return info.Type == this->Type;
      }
  };

  struct is_dead
  {
    bool operator()(boost::shared_ptr<meshserver::common::ExecuteProcess> ep)
      {
      return !ep->isAlive();
      }
  };
}  


namespace meshserver{
namespace broker{

//class that loads all files in the executing directory
//with the msw extension and creates a vector of possible
//meshers with that info
class MSWFinder
{
public:
  typedef std::vector<MeshWorkerInfo>::const_iterator const_iterator;
  typedef std::vector<MeshWorkerInfo>::iterator iterator;
  const std::string MSWExt;


  MSWFinder():
    MSWExt(".MSW")
    {
    boost::filesystem::path cwd = boost::filesystem::current_path();
    this->parseDirectory(cwd);
    }

  MSWFinder(const boost::filesystem::path& path):
    MSWExt(".MSW")
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
        if(ext == MSWExt)
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
      std::string meshType,mesherName;
      f >> meshType;
      f >> mesherName;

      //convert from string to the proper types
      meshserver::MESH_TYPE type = meshserver::to_meshType(meshType);
      boost::filesystem::path p(file.parent_path());
      p /= mesherName;
#ifdef WIN32
      p.replace_extension(".exe");
#endif
      if(boost::filesystem::is_regular_file(p))
        {
        this->Info.push_back(MeshWorkerInfo(type, p.string()));
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
WorkerFactory::WorkerFactory()
{
   MSWFinder finder; //default to current working directory
   this->PossibleWorkers.insert(this->PossibleWorkers.end(),
                                finder.begin(),
                                finder.end());
}

//----------------------------------------------------------------------------
WorkerFactory::~WorkerFactory()
{
}

//----------------------------------------------------------------------------
void WorkerFactory::addWorkerSearchDirectory(const std::string &directory)
{
  boost::filesystem::path dir(directory);
  MSWFinder finder(dir);
  this->PossibleWorkers.insert(this->PossibleWorkers.end(),
                               finder.begin(),
                               finder.end());
}

//----------------------------------------------------------------------------
bool WorkerFactory::haveSupport(meshserver::MESH_TYPE type ) const
{
  support_meshType pred(type);
  WorkerIterator result = std::find_if(this->PossibleWorkers.begin(),
                      this->PossibleWorkers.end(),
                      pred);
  return result != this->PossibleWorkers.end();
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(meshserver::MESH_TYPE type)
{
  this->removeDeadWorkers();
  if(this->CurrentProcesses.size() > 0)
    {
    return false;
    }

  support_meshType pred(type);
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
bool WorkerFactory::addWorker(const std::string& executable)
{
  //add this workers
  std::cout << "adding a worker" << std::endl;
  ExecuteProcessPtr ep(new ExecuteProcess(executable) );
  ep->execute();
  this->CurrentProcesses.push_back(ep);
  return true;
}

//----------------------------------------------------------------------------
void WorkerFactory::removeDeadWorkers()
{
  //foreach current worker remove any that return they are not alive
  ProcessIterator result = std::remove_if(this->CurrentProcesses.begin(),
                                          this->CurrentProcesses.end(),
                                          is_dead());
  this->CurrentProcesses.erase(result,this->CurrentProcesses.end());
}

}
}
