/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_broker_internal_ActiveJobs_h
#define __meshserver_broker_internal_ActiveJobs_h

#include <meshserver/common/zmqHelper.h>

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <meshserver/broker/internal/uuidHelper.h>
#include <meshserver/common/JobResult.h>
#include <meshserver/common/JobStatus.h>

#include <map>

namespace meshserver{
namespace broker{
namespace internal{

class ActiveJobs
{
  public:

    bool add(const zmq::socketAddress& workerAddress, const boost::uuids::uuid& id);

    bool remove(const boost::uuids::uuid& id);

    bool haveUUID(const boost::uuids::uuid& id) const;

    bool haveResult(const boost::uuids::uuid& id) const;

    const meshserver::common::JobStatus& status(const boost::uuids::uuid& id);

    const meshserver::common::JobResult& result(const boost::uuids::uuid& id);

    void updateStatus(const meshserver::common::JobStatus& s);

    void updateResult(const meshserver::common::JobResult& r);

    void markFailedJobs(const boost::posix_time::ptime& time);

    void refreshJobs(const zmq::socketAddress &workerAddress);

private:
    struct JobState
    {
      zmq::socketAddress WorkerAddress;
      meshserver::common::JobStatus jstatus;
      meshserver::common::JobResult jresult;
      boost::posix_time::ptime expiry; //after this time the job should be purged
      bool haveResult;

      JobState(const zmq::socketAddress& workerAddress,
               const boost::uuids::uuid& id,
               meshserver::STATUS_TYPE stat):
        WorkerAddress(workerAddress),
        jstatus(id,stat),
        jresult(id),
        expiry(boost::posix_time::second_clock::local_time()),
        haveResult(false)
        {
          //we give it two heartbeat cycles of lifetime to start
          expiry = expiry + boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC);
        }

        void refresh()
        {
          expiry = boost::posix_time::second_clock::local_time() +
            boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC);
        }

        //we can only change the status of jobs that are
        //IN_PROGRESS or QUEUED. If they are finished or failed it is pointless
        bool canUpdateStatus() const
        {
          return jstatus.Status == QUEUED || jstatus.Status == IN_PROGRESS;
        }
    };

    typedef std::pair<boost::uuids::uuid, JobState> InfoPair;
    typedef std::map< boost::uuids::uuid, JobState>::const_iterator InfoConstIt;
    typedef std::map< boost::uuids::uuid, JobState>::iterator InfoIt;
    std::map<boost::uuids::uuid, JobState> Info;
};

//-----------------------------------------------------------------------------
bool ActiveJobs::add(const zmq::socketAddress &workerAddress,const boost::uuids::uuid& id)
{
  if(!this->haveUUID(id))
    {
    JobState ws(workerAddress,id,meshserver::QUEUED);
    InfoPair pair(id,ws);
    this->Info.insert(pair);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool ActiveJobs::remove(const boost::uuids::uuid& id)
{
  if(this->haveUUID(id))
    {
    this->Info.erase(id);
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool ActiveJobs::haveUUID(const boost::uuids::uuid& id) const
{
  return this->Info.count(id) != 0;
}

//-----------------------------------------------------------------------------
bool ActiveJobs::haveResult(const boost::uuids::uuid& id) const
{
  InfoConstIt item = this->Info.find(id);
  if(item == this->Info.end())
    {
    return false;
    }
  return item->second.haveResult;
}

//-----------------------------------------------------------------------------
const meshserver::common::JobStatus& ActiveJobs::status(
     const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jstatus;
}

//-----------------------------------------------------------------------------
const common::JobResult& ActiveJobs::result(
    const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jresult;
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateStatus(const meshserver::common::JobStatus& s)
{
  InfoIt item = this->Info.find(s.JobId);

  if(item->second.canUpdateStatus())
    {
    item->second.jstatus = s;
    item->second.refresh();
    }
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateResult(const meshserver::common::JobResult& r)
{
  InfoIt item = this->Info.find(r.JobId);
  item->second.jresult = r;
  item->second.haveResult = true;
  item->second.refresh();
}

//-----------------------------------------------------------------------------
void ActiveJobs::markFailedJobs(const boost::posix_time::ptime& time)
{
  for(InfoIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    //we can only mark jobs that are IN_PROGRESS or QUEUED as failed.
    //FINISHED is more important than failed
    if (item->second.canUpdateStatus() && item->second.expiry < time)
      {
      item->second.jstatus.Status = meshserver::FAILED;
      item->second.jstatus.Progress = 0;
      std::cout << "Marking job id: " << item->first << " as FAILED" << std::endl;
      }
    }
}

//-----------------------------------------------------------------------------
void ActiveJobs::refreshJobs(const zmq::socketAddress& workerAddress)
{
  for(InfoIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    if(item->second.WorkerAddress == workerAddress)
      {
      item->second.refresh();
      }
    }
}

}
}
}

#endif
