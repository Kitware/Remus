/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal_WorkeryFactory_h
#define __meshserver_broker_internal_WorkeryFactory_h

#include <queue>
#include <set>

#include <meshserver/common/meshServerGlobals.h>b

namespace meshserver{
namespace broker{
namespace internal{

class WorkerFactory
{
  public:
    bool haveSupport(meshserver::MESH_TYPE type ) const { return true; }

    bool createWorker(meshserver::MESH_TYPE type){return true;}


};

}
}
}

#endif
