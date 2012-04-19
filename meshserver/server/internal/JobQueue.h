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

#ifndef __meshserver_server_internal_JobQueue_h
#define __meshserver_server_internal_JobQueue_h

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>
#include <set>

#include <meshserver/server/internal/uuidHelper.h>
#include <meshserver/common/JobDetails.h>
#include <meshserver/JobMessage.h>

namespace meshserver{
namespace server{
namespace internal{

//A FIFO queue. each mesh type has its own queue
//where we keep jobs

class JobQueue
{
public:
  JobQueue():Queue(){}

  //Convert a JobMessage and UUID into a WorkerMessage.
  bool addJob( const boost::uuids::uuid& id,
             const meshserver::JobMessage& message);

  //Removes a job from the queue of the given mesh type.
  //Return it as a JobDetail
  meshserver::common::JobDetails takeJob(meshserver::MESH_TYPE type);

  //returns the types of jobs that are waiting for a worker
  std::set<meshserver::MESH_TYPE> waitingForWorkerTypes() const;

  //returns the types of jobs that are queued and aren't waiting for a woker
  std::set<meshserver::MESH_TYPE> queuedJobTypes() const;

  //marks the first job with the given type as having
  //a worker dispatched for it.
  bool workerDispatched(meshserver::MESH_TYPE type);

  //Returns true if we contain the UUID
  bool haveUUID(const boost::uuids::uuid& id) const;

private:
  struct QueuedJob
  {
    QueuedJob(const boost::uuids::uuid& id,
              const meshserver::JobMessage& message):
              Id(id),
              Message(message),
              WaitingForWorker(false),
              WorkerDispatchTime(boost::posix_time::second_clock::local_time())
              {}

    boost::uuids::uuid Id;
    meshserver::JobMessage Message;

    //information on when the job was marked as scheduled
    bool WaitingForWorker;
    boost::posix_time::ptime WorkerDispatchTime;

  };

  //job queue
  std::vector<QueuedJob> Queue;

  //quick lookup of all ids in job queue
  std::set<boost::uuids::uuid> QueuedIds;

  //make copying not possible
  JobQueue (const JobQueue&);
  void operator = (const JobQueue&);
};


//------------------------------------------------------------------------------
bool JobQueue::addJob(const boost::uuids::uuid &id,
                    const meshserver::JobMessage& message)
{
  this->Queue.push_back(QueuedJob(id,message));
  this->QueuedIds.insert(id);
}

//------------------------------------------------------------------------------
meshserver::common::JobDetails JobQueue::takeJob(MESH_TYPE type)
{
  QueuedJob* job;
  int index = 0;
  bool found = false;
  for(index=0; index < this->Queue.size(); ++index)
    {
    job = &this->Queue[index];
    if(job->Message.meshType()==type)
      {
      found = true;
      break;
      }
    }

  if(found)
    {
    boost::uuids::uuid id = job->Id;
    //convert the queued job into a job detail
    const std::string data(job->Message.data(),job->Message.dataSize());

    this->QueuedIds.erase(id);
    this->Queue.erase(this->Queue.begin()+index);
    std::cout << "num queued jobs left: " << this->Queue.size() << std::endl;

    return meshserver::common::JobDetails(id,data);
    }

  std::cout << "returning fake job" << std::endl;
  return meshserver::common::JobDetails(boost::uuids::uuid(),"INVALID");
}

//------------------------------------------------------------------------------
std::set<meshserver::MESH_TYPE> JobQueue::waitingForWorkerTypes() const
{
  typedef std::vector<QueuedJob>::const_iterator it;
  std::set<meshserver::MESH_TYPE> types;
  for(it i = this->Queue.begin(); i != this->Queue.end(); ++i)
    {
    const QueuedJob& job = *i;
    if(job.WaitingForWorker)
      {
      types.insert(job.Message.meshType());
      }
    }
  return types;
}

//------------------------------------------------------------------------------
std::set<meshserver::MESH_TYPE> JobQueue::queuedJobTypes() const
{
  typedef std::vector<QueuedJob>::const_iterator it;
  std::set<meshserver::MESH_TYPE> types;
  for(it i = this->Queue.begin(); i != this->Queue.end(); ++i)
    {
    const QueuedJob& job = *i;
    if(!job.WaitingForWorker)
      {
      types.insert(job.Message.meshType());
      }
    }
  return types;
}

//------------------------------------------------------------------------------
bool JobQueue::workerDispatched(meshserver::MESH_TYPE type)
{
  bool found = false;
  typedef std::vector<QueuedJob>::iterator it;
  for(it i = this->Queue.begin(); i != this->Queue.end() && !found; ++i)
    {
    QueuedJob& job = *i;
    //find the first work of a given mesh type that isn't already
    //waiting for a worker
    if(job.Message.meshType() == type && !job.WaitingForWorker)
      {
      job.WaitingForWorker = true;
      job.WorkerDispatchTime = boost::posix_time::second_clock::local_time();
      found = true;
      }
    }
  return found;
}

//------------------------------------------------------------------------------
bool JobQueue::haveUUID(const boost::uuids::uuid &id) const
{
  return this->QueuedIds.count(id) == 1 ? true : false;
}

}
}
} //namespace meshserver::server::internal

#endif // JOBQUEUE_H
