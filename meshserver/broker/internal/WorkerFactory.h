/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal_WorkeryFactory_h
#define __meshserver_broker_internal_WorkeryFactory_h

#include <meshserver/common/meshServerGlobals.h>
#include <boost/scoped_pointer.hpp>


namespace meshserver{
namespace broker{
namespace internal{

class MSWFinder;

struct MeshWorkerInfo
{
  meshserver::MESH_TYPE;
  std::string ExecutionPath;
};

class WorkerFactory
{
  public:
  	WorkerFactory();
    bool haveSupport(meshserver::MESH_TYPE type );

    bool createWorker(meshserver::MESH_TYPE type);
private:
  bool launchWorkerProcess(const MeshWorkerInfo& worker);

  boost::scoped_pointer<MSWFinder> FileFinder;
  std::vector<MeshWorkerInfo> PossibleWorkers;
};

}
}
}

#endif
