/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_WorkeryFactory_h
#define __meshserver_broker_WorkeryFactory_h

#include <meshserver/common/meshServerGlobals.h>
#include <vector>

namespace meshserver{
namespace broker{

struct MeshWorkerInfo
{
  meshserver::MESH_TYPE Type;
  std::string ExecutionPath;
  MeshWorkerInfo(meshserver::MESH_TYPE t, const std::string& p):
    Type(t),ExecutionPath(p){}
};

class WorkerFactory
{
  public:
  	WorkerFactory();
    virtual ~WorkerFactory();

    //add a path to search for workers.
    //by default we only search the current working directory
    void addWorkerSearchDirectory(const std::string& directory);

    virtual bool haveSupport(meshserver::MESH_TYPE type ) const;
    virtual bool createWorker(meshserver::MESH_TYPE type) const;
private:
  std::vector<MeshWorkerInfo> PossibleWorkers;
};

}
}

#endif
