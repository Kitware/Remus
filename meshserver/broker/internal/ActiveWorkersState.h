/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal_ActiveWorkerState_h
#define __meshserver_broker_internal_ActiveWorkerState_h

#include <queue>
#include <set>

#include <boost/uuid/uuid.hpp>
#include <meshserver/broker/internal/uuidHelper.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

namespace meshserver{
namespace broker{
namespace internal{

class ActiveWorkersState
{
  public:
    bool haveUUID(const boost::uuids::uuid& id)
      { return true; }

    bool jobFinished(const boost::uuids::uuid& id)
      { return true; }

    meshserver::STATUS_TYPE status(const boost::uuids::uuid& id)
      { return meshserver::IN_PROGRESS; }

    meshserver::common::JobResult result(const boost::uuids::uuid& id)
    { return meshserver::common::JobResult(meshserver::to_string(id)); }

    void updateStatus(const meshserver::common::JobStatus& status)
    { }

    void updateResult(const meshserver::common::JobResult& result)
    { }
};

}
}
}

#endif
