/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/internal/WorkerFactory.h>

namespace meshserver{
namespace broker{
namespace internal{

class MSWFinder::MSWFinder
{

};

//----------------------------------------------------------------------------
WorkerFactory::WorkerFactory()
{

}

//----------------------------------------------------------------------------
bool WorkerFactory::haveSupport(meshserver::MESH_TYPE type ) const
{
  return true;
}

//----------------------------------------------------------------------------
bool WorkerFactory::createWorker(meshserver::MESH_TYPE type)
{
  return true;
}

//----------------------------------------------------------------------------
bool WorkerFactory::launchWorkerProcess(const MeshWorkerInfo& worker)
{

}


}
}
}
