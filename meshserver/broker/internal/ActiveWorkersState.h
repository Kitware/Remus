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

      workerState(const boost::uuids::uuid& id, meshserver::STATUS_TYPE stat):
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
    workerState ws(id,meshserver::QUEUED);
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
  InfoIt item = this->Info.find(s.JobId);

  //The status enum is numbered in such away that a lower status value
  //can't be reached from a higher status value. For example FAILED is status
  //value 4, and can't be overwritten by status value 2 which is IN_PROGRESS.
  //Since we are in asnyc land we could get a 4 than a 2 when a worker is crashing
  if(s.Status >= item->second.jstatus.Status)
    {
    item->second.jstatus = s;
    }
}

//-----------------------------------------------------------------------------
void ActiveWorkersState::updateResult(const meshserver::common::JobResult& r)
{
  InfoIt item = this->Info.find(r.JobId);
  item->second.jresult = r;
}

}
}
}

#endif
