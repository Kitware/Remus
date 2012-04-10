/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/internal/WorkerFactory.h>
#include <meshserver/common/ExecuteProcess.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <fstream>

namespace
{
  typedef std::vector<meshserver::broker::internal::MeshWorkerInfo>::const_iterator WorkerIterator;

  struct support_meshType
  {
    meshserver::MESH_TYPE Type;
    support_meshType(meshserver::MESH_TYPE type):Type(type){}
    bool operator()(const meshserver::broker::internal::MeshWorkerInfo& info)
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
  const std::string MSWExt( "MSW" );
  boost::filesystem::path cwd = boost::filesystem::current_path();

  boost::filesystem::directory_iterator end_itr;
  for( boost::filesystem::directory_iterator i( cwd ); i != end_itr; ++i )
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

  //now that we have all the files parse each one  
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
      boost::filesystem::path p(file.string());
      p /= mesherName;


      std::cout << p.string() << std::endl;
      this->Info.push_back(MeshWorkerInfo(type, p.string()));
      }
    f.close();

  }

  const std::vector<MeshWorkerInfo>& results(){return Info;}

private:
  std::vector<MeshWorkerInfo> Info;  

};

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory():
 FileFinder(new MSWFinder())
{
  this->PossibleWorkers = this->FileFinder->results();
}

//----------------------------------------------------------------------------
WorkerFactory::~WorkerFactory()
{
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
bool WorkerFactory::createWorker(meshserver::MESH_TYPE type) const
{
  support_meshType pred(type);
  WorkerIterator result = std::find_if(this->PossibleWorkers.begin(),
                      this->PossibleWorkers.end(),
                      pred);

  if(result == this->PossibleWorkers.end())
    {
    return false;
    }

  meshserver::common::ExecuteProcess ep((*result).ExecutionPath);
  ep.execute(true);
  return true;
}

}
}
}

