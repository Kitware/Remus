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

#include <remus/server/detail/ActiveJobs.h>

#include <remus/server/detail/uuidHelper.h>

namespace remus{
namespace server{
namespace detail{

//-----------------------------------------------------------------------------
ActiveJobs::JobState::JobState(const zmq::SocketIdentity& workerIdentity,
         const boost::uuids::uuid& id,
         remus::STATUS_TYPE stat):
  WorkerAddress(workerIdentity),
  jstatus(id,stat),
  jresult(id),
  haveResult(false)
{

}

//-----------------------------------------------------------------------------
bool ActiveJobs::JobState::canUpdateStatusTo(remus::proto::JobStatus s) const
{
  //we don't want the worker to ever explicitly state it has finished the
  //job. We want that state to only be applied when the results have finished
  //transferring to the server
  //we can only change the status of jobs that are IN_PROGRESS or QUEUED.
  //If the job status is finished
  //If we are in IN_PROGRESS we can't move back to QUEUED
  return jstatus.good() && !s.finished() && (s.failed() || s.inProgress());
}

//-----------------------------------------------------------------------------
bool ActiveJobs::add(const zmq::SocketIdentity &workerIdentity,
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
zmq::SocketIdentity ActiveJobs::workerAddress(
                                          const boost::uuids::uuid& id) const
{
  InfoConstIt item = this->Info.find(id);
  if(item == this->Info.end())
    {
    return zmq::SocketIdentity();
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
const remus::proto::JobStatus& ActiveJobs::status(
     const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jstatus;
}

//-----------------------------------------------------------------------------
const remus::proto::JobResult& ActiveJobs::result(
    const boost::uuids::uuid& id)
{
  InfoConstIt item = this->Info.find(id);
  return item->second.jresult;
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateStatus(const remus::proto::JobStatus& s)
{
  InfoIt item = this->Info.find(s.id());

  if(item != this->Info.end() && item->second.canUpdateStatusTo(s) )
    {
    //we don't want the worker to ever explicitly state it has finished the
    //job. That is why we use canUpdateStatusTo, which checks the status
    //we are moving too
    item->second.jstatus = s;
    }
}

//-----------------------------------------------------------------------------
void ActiveJobs::updateResult(const remus::proto::JobResult& r)
{
  InfoIt item = this->Info.find(r.id());
  if(item != this->Info.end())
    {
    //once we get a result we can state our status is now finished,
    //since the uploading of data has finished.
    if( item->second.jstatus.status() != remus::FAILED )
      {
      item->second.jstatus = remus::proto::JobStatus(r.id(),remus::FINISHED);
      }

    //update the client result data to equal the server data
    item->second.jresult = r;
    item->second.haveResult = true;
    }
}

//-----------------------------------------------------------------------------
std::vector< remus::proto::JobStatus >
ActiveJobs::markExpiredJobs(remus::server::detail::SocketMonitor monitor)
{
  std::vector< remus::proto::JobStatus > expiredJobs;
  for(InfoIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    //we can only mark jobs that are IN_PROGRESS or QUEUED as failed.
    //FINISHED is more important than failed
    const bool is_status_valid_to_expire = (item->second.jstatus.queued() ||
                                           item->second.jstatus.inProgress());
    const bool worker_is_unresponsive = monitor.isUnresponsive(
                                                  item->second.WorkerAddress);
    if (is_status_valid_to_expire && worker_is_unresponsive)
      {
      //marking the job status as expired
      item->second.jstatus =
          remus::proto::JobStatus( item->second.jstatus.id(),remus::EXPIRED);
      expiredJobs.push_back( item->second.jstatus );
      }
    }
  return expiredJobs;
}

//-----------------------------------------------------------------------------
std::set<zmq::SocketIdentity> ActiveJobs::activeWorkers() const
{
  std::set<zmq::SocketIdentity> workerAddresses;
  for(InfoConstIt item = this->Info.begin(); item != this->Info.end(); ++item)
    {
    workerAddresses.insert(item->second.WorkerAddress);
    }
  return workerAddresses;
}


}
}
}
