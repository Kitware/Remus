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

#include <map>

namespace meshserver{
namespace broker{
namespace internal{

class ActiveWorkersState
{
  public:

    bool add(const boost::uuids::uuid& id);

    bool remove(const boost::uuids::uuid& id);

    bool haveUUID(const boost::uuids::uuid& id) const;

    bool haveResult(const boost::uuids::uuid& id) const;

    const meshserver::common::JobStatus& status(const boost::uuids::uuid& id);

    const meshserver::common::JobResult& result(const boost::uuids::uuid& id);

    void updateStatus(const meshserver::common::JobStatus& s);

    void updateResult(const meshserver::common::JobResult& r);

private:
    struct workerState
    {
      bool haveStatus;
      bool haveResult;
      meshserver::common::JobStatus jstatus;
      meshserver::common::JobResult jresult;

      workerState(const std::string& id, meshserver::STATUS_TYPE stat):
        haveStatus(false),haveResult(false),
        jstatus(id,stat),jresult(id)
      {}
    };

    typedef std::pair<boost::uuids::uuid, workerState> InfoPair;
    typedef std::map< boost::uuids::uuid, workerState>::const_iterator InfoConstIt;
    typedef std::map< boost::uuids::uuid, workerState>::iterator InfoIt;
    std::map<boost::uuids::uuid, workerState> Info;
};

//-----------------------------------------------------------------------------
bool ActiveWorkersState::add(const boost::uuids::uuid& id)
{
  if(!this->haveUUID(id))
    {
    workerState ws(meshserver::to_string(id),meshserver::QUEUED);
    InfoPair pair(id,ws);
    this->Info.insert(pair);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool ActiveWorkersState::remove(const boost::uuids::uuid& id)
{
  if(this->haveUUID(id))
    {
    this->Info.erase(id);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool ActiveWorkersState::haveUUID(const boost::uuids::uuid& id) const
{
  return this->Info.count(id) != 0;
}

//-----------------------------------------------------------------------------
bool ActiveWorkersState::haveResult(const boost::uuids::uuid& id) const
{
  InfoConstIt item = this->Info.find(id);
  if(item == this->Info.end())
    {
    return false;
    }
  return item->second.haveResult;
}

//-----------------------------------------------------------------------------
const meshserver::common::JobStatus& ActiveWorkersState::status(
     const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jstatus;
}

//-----------------------------------------------------------------------------
const common::JobResult& ActiveWorkersState::result(
    const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jresult;
}

//-----------------------------------------------------------------------------
void ActiveWorkersState::updateStatus(const meshserver::common::JobStatus& s)
{
  InfoIt item = this->Info.find(meshserver::to_uuid(s.JobId));
  item->second.jstatus = s;
}

//-----------------------------------------------------------------------------
void ActiveWorkersState::updateResult(const meshserver::common::JobResult& r)
{
  InfoIt item = this->Info.find(meshserver::to_uuid(r.JobId));
  item->second.jresult = r;
}

}
}
}

#endif
