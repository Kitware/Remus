/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/internal/WorkerFactory.h>
#include <meshserver/common/ExecuteProcess.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>


#include <algorithm>

namespace
{
  typedef std::vector<MeshWorkerInfo>::iterator WorkerIterator;

  struct support_meshType
  {
    meshserver::MESH_TYPE Type;
    support_meshType(meshserver::MESH_TYPE type):Type(type);
    bool operator()(const MeshWorkerInfo& info)
      {
      return info.Type == this->Type;
      }
  };
}  


namespace meshserver{
namespace broker{
namespace internal{

//class that loads all files in the executing directory
//with the msw extension and creates a vector of possible
//meshers with that info
class MSWFinder
{
public:
  MSWFinder()
  {
  const boost::regex filter( ".*\.msw" );
  boost::filesystem::path cwd = boost::filesystem::current_path();

  boost::filesystem::directory_iterator end_itr;
  boost::smatch match;
  for( boost::filesystem::directory_iterator i( cwd ); i != end_itr; ++i )
    {
    // Skip if not a file
    if(boost::filesystem::is_regular_file( i->status() ) &&
       boost::regex_match( i->leaf(), match, filter ))
      {
      this->parseFile(i->leaf());
      }
    }

  //now that we have all the files parse each one  
  }

  void parseFile(const std::string& file)
  {
    //open the file, parse two lines and close file
    boost::filesystem::path cwd = boost::filesystem::current_path();
    fstream f(file,"r");
    if(f.open())
    {
      std::string type,name;
      boost::filesystem::path p(cwd);
      file >> type;
      file >> name;
      p /= name;
      this->Info.push_back(MeshWorkerInfo(type, p.string()));
    }

  }

  const std::vector<MeshWorkerInfo>& results();

private:
  std::vector<MeshWorkerInfo> Info;  

};

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory():
FileFinder()
{
  this->PossibleWorkers = this->FileFinder.results();
}

//----------------------------------------------------------------------------
bool WorkerFactory::haveSupport(meshserver::MESH_TYPE type ) const
{
  support_meshType pred(type);
  return std::find_if(this->PossibleWorkers.begin(),
                      this->PossibleWorkers.end(),
                      pred) != this->PossibleWorkers.end();
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(meshserver::MESH_TYPE type)
{
  support_meshType pred(type);
  WorkerIterator result = std::find_if(this->PossibleWorkers.begin(),
                      this->PossibleWorkers.end(),
                      pred);

  if(result == this->PossibleWorkers.end())
    {
    return false;
    }

  meshserver::common::ExecuteProcess ep(result.ExecutionPath);
  ep.execute(true);
  return true;
}

}
}
}

#endif
