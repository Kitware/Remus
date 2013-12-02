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

#include <remus/common/zmqHelper.h>

#include <remus/client/JobResult.h>
#include <remus/client/JobStatus.h>
#include <remus/worker/JobResult.h>
#include <remus/worker/JobStatus.h>

#include <remus/server/internal/uuidHelper.h>
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

    inline bool add(const zmq::socketIdentity& workerIdentity,
                    const boost::uuids::uuid& id);

    inline bool remove(const boost::uuids::uuid& id);

    inline zmq::socketIdentity workerAddress(const boost::uuids::uuid& id) const;

    inline bool haveUUID(const boost::uuids::uuid& id) const;

    inline bool haveResult(const boost::uuids::uuid& id) const;

    //returns a worker side job status object for a job
    inline const remus::client::JobStatus& status(const boost::uuids::uuid& id);

    //returns a worker side job result object for a job
    inline const remus::client::JobResult& result(const boost::uuids::uuid& id);

    //update the job status of a job.
    //valid values are:
    // QUEUED
    // IN_PROGRESS
    // FAILED
    // EXPIRED
    // To update a job to the finished state, you have to call updateResult
    // not update status
    inline void updateStatus(const remus::worker::JobStatus& s);

    inline void updateResult(const remus::worker::JobResult& r);

    inline void markExpiredJobs(const boost::posix_time::ptime& time);

    inline void refreshJobs(const zmq::socketIdentity &workerIdentity);

    inline std::set<zmq::socketIdentity> activeWorkers() const;

private:
    struct JobState
    {
      zmq::socketIdentity WorkerAddress;
      remus::client::JobStatus jstatus;
      remus::client::JobResult jresult;
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

        bool canUpdateStatusTo(remus::worker::JobStatus s) const
        {
        //we don't want the worker to ever explicitly state it has finished the
        //job. We want that state to only be applied when the results have finished
        //transferring to the server
        //we can only change the status of jobs that are IN_PROGRESS or QUEUED.
        //If the job status is finished
        //If we are in IN_PROGRESS we can't move back to QUEUED
        return jstatus.good() && !s.finished() && (s.failed() || s.inProgress());
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
const remus::client::JobStatus& ActiveJobs::status(
     const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jstatus;
}

//-----------------------------------------------------------------------------
const remus::client::JobResult& ActiveJobs::result(
    const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jresult;
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateStatus(const remus::worker::JobStatus& s)
{
  InfoIt item = this->Info.find(s.JobId);

  if(item != this->Info.end() && item->second.canUpdateStatusTo(s) )
    {
    //we don't want the worker to ever explicitly state it has finished the
    //job. That is why we use canUpdateStatusTo, which checks the status
    //we are moving too
    item->second.jstatus.Status = s.Status;
    item->second.jstatus.Progress = s.Progress;
    }
  item->second.refresh();
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateResult(const remus::worker::JobResult& r)
{
  InfoIt item = this->Info.find(r.JobId);
  if(item != this->Info.end())
    {
    //once we get a result we can state our status is now finished,
    //since the uploading of data has finished.
    if( item->second.jstatus.good() || item->second.jstatus.finished() )
      {
      item->second.jstatus = remus::client::JobStatus(r.JobId,remus::FINISHED);
      }

    //update the client result data to equal the server data
    item->second.jresult.Data = r.Data;
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
    if ( !(item->second.jstatus.failed() || item->second.jstatus.finished()) &&
           item->second.expiry < time)
      {
      item->second.jstatus.Status = remus::EXPIRED;
      item->second.jstatus.Progress =
                                remus::client::JobProgress(remus::EXPIRED);
      //std::cout << "Marking job id: " << item->first << " as EXPIRED" << std::endl;
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
