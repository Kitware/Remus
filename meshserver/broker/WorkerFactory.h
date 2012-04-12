/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_WorkeryFactory_h
#define __meshserver_broker_WorkeryFactory_h

#include <vector>
#include <meshserver/common/meshServerGlobals.h>
#include <boost/shared_ptr.hpp>

//forward declare the execute process
namespace meshserver{
namespace common{
class ExecuteProcess;
}
}

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
  //Work Factory defaults to creating a maximum of 1 workers at once
  WorkerFactory();
  virtual ~WorkerFactory();

  //add a path to search for workers.
  //by default we only search the current working directory
  void addWorkerSearchDirectory(const std::string& directory);

  virtual bool haveSupport(meshserver::MESH_TYPE type ) const;
  virtual bool createWorker(meshserver::MESH_TYPE type);

  //checks all current processes and removes any that have
  //shutdown
  virtual void updateWorkerCount();

  //Set the maximum number of total workers that can be returning at once
  void setMaxWorkerCount(unsigned int count){MaxWorkers = count;}
  unsigned int maxWorkerCount(){return MaxWorkers;}
  unsigned int currentWorkerCount() const { return this->CurrentProcesses.size(); }


protected:
  typedef meshserver::common::ExecuteProcess ExecuteProcess;
  typedef boost::shared_ptr<ExecuteProcess> ExecuteProcessPtr;

  virtual bool addWorker(const std::string& executable);

  unsigned int MaxWorkers;
  std::vector<MeshWorkerInfo> PossibleWorkers;
  std::vector<ExecuteProcessPtr> CurrentProcesses;
};

}
}

#endif
