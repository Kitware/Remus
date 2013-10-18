//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef __remus_server_internal_ActiveJobs_h
#define __remus_server_internal_ActiveJobs_h

#include <remus/JobResult.h>
#include <remus/JobStatus.h>
#include <remus/common/zmqHelper.h>
#include <remus/server/internal/uuidHelper.h>

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <map>
#include <set>

namespace remus{
namespace server{
namespace internal{

class ActiveJobs
{
  public:
    ActiveJobs():Info(){}

    bool add(const zmq::socketIdentity& workerIdentity,
             const boost::uuids::uuid& id);

    bool remove(const boost::uuids::uuid& id);

    zmq::socketIdentity workerAddress(const boost::uuids::uuid& id) const;

    bool haveUUID(const boost::uuids::uuid& id) const;

    bool haveResult(const boost::uuids::uuid& id) const;

    const remus::JobStatus& status(const boost::uuids::uuid& id);

    const remus::JobResult& result(const boost::uuids::uuid& id);

    void updateStatus(const remus::JobStatus& s);

    void updateResult(const remus::JobResult& r);

    void markExpiredJobs(const boost::posix_time::ptime& time);

    void refreshJobs(const zmq::socketIdentity &workerIdentity);

    std::set<zmq::socketIdentity> activeWorkers() const;

private:
    struct JobState
    {
      zmq::socketIdentity WorkerAddress;
      remus::JobStatus jstatus;
      remus::JobResult jresult;
      boost::posix_time::ptime expiry; //after this time the job should be purged
      bool haveResult;

      JobState(const zmq::socketIdentity& workerIdentity,
               const boost::uuids::uuid& id,
               remus::STATUS_TYPE stat):
        WorkerAddress(workerIdentity),
        jstatus(id,stat),
        jresult(id),
        expiry(boost::posix_time::second_clock::local_time()),
        haveResult(false)
        {
          //we give it two heartbeat cycles of lifetime to start
          expiry += boost::posix_time::seconds(HEARTBEAT_INTERVAL_IN_SEC*2);
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
bool ActiveJobs::add(const zmq::socketIdentity &workerIdentity,
                     const boost::uuids::uuid& id)
{
  if(!this->haveUUID(id))
    {
    JobState ws(workerIdentity,id,remus::QUEUED);
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
zmq::socketIdentity ActiveJobs::workerAddress(
                                          const boost::uuids::uuid& id) const
{
  InfoConstIt item = this->Info.find(id);
  if(item == this->Info.end())
    {
    return zmq::socketIdentity();
    }
  return item->second.WorkerAddress;
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
const remus::JobStatus& ActiveJobs::status(
     const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jstatus;
}

//-----------------------------------------------------------------------------
const remus::JobResult& ActiveJobs::result(
    const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jresult;
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateStatus(const remus::JobStatus& s)
{
  InfoIt item = this->Info.find(s.JobId);

  if(item != this->Info.end() &&
     item->second.canUpdateStatus())
    {
    //we don't want the worker to ever explicitly state it has finished the
    //job. We want that state to only be applied when the results have finished
    //transferring to the server
    if(!s.finished())
      {
      item->second.jstatus = s;
      }
    item->second.refresh();
    }
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateResult(const remus::JobResult& r)
{
  InfoIt item = this->Info.find(r.JobId);
  if(item != this->Info.end())
    {
    //once we get a result we can state our status is now finished,
    //since the uploading of data has finished.
    if(item->second.canUpdateStatus())
      {
      item->second.jstatus = remus::JobStatus(r.JobId,remus::FINISHED);
      }
    item->second.jresult = r;
    item->second.haveResult = true;
    item->second.refresh();
    }
}

//-----------------------------------------------------------------------------
void ActiveJobs::markExpiredJobs(const boost::posix_time::ptime& time)
{
  for(InfoIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    //we can only mark jobs that are IN_PROGRESS or QUEUED as failed.
    //FINISHED is more important than failed
    if (item->second.canUpdateStatus() && item->second.expiry < time)
      {
      item->second.jstatus.Status = remus::EXPIRED;
      item->second.jstatus.Progress = remus::JobProgress(remus::EXPIRED);
      //std::cout << "Marking job id: " << item->first << " as FAILED" << std::endl;
      }
    }
}

//-----------------------------------------------------------------------------
void ActiveJobs::refreshJobs(const zmq::socketIdentity& workerIdentity)
{
  for(InfoIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    if(item->second.WorkerAddress == workerIdentity)
      {
      item->second.refresh();
      }
    }
}

//-----------------------------------------------------------------------------
std::set<zmq::socketIdentity> ActiveJobs::activeWorkers() const
{
  std::set<zmq::socketIdentity> workerAddresses;
  for(InfoConstIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    workerAddresses.insert(item->second.WorkerAddress);
    }
  return workerAddresses;
}


}
}
}

#endif
