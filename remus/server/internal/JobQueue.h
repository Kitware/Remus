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

#ifndef __remus_server_internal_JobQueue_h
#define __remus_server_internal_JobQueue_h

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <vector>
#include <set>

#include <remus/Job.h>
#include <remus/JobRequest.h>
#include <remus/common/Message.h>
#include <remus/server/internal/uuidHelper.h>

namespace remus{
namespace server{
namespace internal{

//A FIFO queue. each mesh type has its own queue
//where we keep jobs

class JobQueue
{
public:
  JobQueue():Queue(){}

  //Convert a Message and UUID into a WorkerMessage.
  bool addJob( const boost::uuids::uuid& id,
             const remus::common::Message& message);

  //Removes a job from the queue of the given mesh type.
  //Return it as a Job
  remus::Job takeJob(remus::common::MeshIOType type);

  //returns the types of jobs that are waiting for a worker
  std::set<remus::common::MeshIOType> waitingForWorkerTypes() const;

  //returns the types of jobs that are queued and aren't waiting for a woker
  std::set<remus::common::MeshIOType> queuedJobTypes() const;

  //marks the first job with the given type as having
  //a worker dispatched for it.
  bool workerDispatched(remus::common::MeshIOType type);

  //Returns true if we contain the UUID
  bool haveUUID(const boost::uuids::uuid& id) const;

  //Returns true if we can remove a job with a give uuid
  bool remove(const boost::uuids::uuid& id);

private:
  struct QueuedJob
  {
    QueuedJob(const boost::uuids::uuid& id,
              const remus::common::Message& message):
              Id(id),
              Message(message),
              WaitingForWorker(false),
              WorkerDispatchTime(boost::posix_time::second_clock::local_time())
              {}

    boost::uuids::uuid Id;
    remus::common::Message Message;

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
                    const remus::common::Message& message)
{
  this->Queue.push_back(QueuedJob(id,message));
  this->QueuedIds.insert(id);
  return true;
}

//------------------------------------------------------------------------------
remus::Job JobQueue::takeJob(remus::common::MeshIOType type)
{
  QueuedJob* queued;
  int index = 0;
  bool found = false;
  for(index=0; index < this->Queue.size(); ++index)
    {
    queued = &this->Queue[index];
    if(queued->Message.MeshIOType()==type)
      {
      found = true;
      break;
      }
    }

  if(found)
    {
    boost::uuids::uuid id = queued->Id;

    //todo this needs to be easier to convert an encoded job request
    //into the job that is being sent to the worker, without having to copy
    //the data so many times.
    const remus::JobRequest request(remus::to_JobRequest(queued->Message.data(),
                                                         queued->Message.dataSize()));
    remus::Job job(id,type,request.jobInfo());

    this->QueuedIds.erase(id);
    this->Queue.erase(this->Queue.begin()+index);
    return job;
    }
  return remus::Job();
}

//------------------------------------------------------------------------------
std::set<remus::common::MeshIOType> JobQueue::waitingForWorkerTypes() const
{
  typedef std::vector<QueuedJob>::const_iterator it;
  std::set<remus::common::MeshIOType> types;
  for(it i = this->Queue.begin(); i != this->Queue.end(); ++i)
    {
    const QueuedJob& job = *i;
    if(job.WaitingForWorker)
      {
      types.insert(job.Message.MeshIOType());
      }
    }
  return types;
}

//------------------------------------------------------------------------------
std::set<remus::common::MeshIOType> JobQueue::queuedJobTypes() const
{
  typedef std::vector<QueuedJob>::const_iterator it;
  std::set<remus::common::MeshIOType> types;
  for(it i = this->Queue.begin(); i != this->Queue.end(); ++i)
    {
    const QueuedJob& job = *i;
    if(!job.WaitingForWorker)
      {
      types.insert(job.Message.MeshIOType());
      }
    }
  return types;
}

//------------------------------------------------------------------------------
bool JobQueue::workerDispatched(remus::common::MeshIOType type)
{
  bool found = false;
  typedef std::vector<QueuedJob>::iterator it;
  for(it i = this->Queue.begin(); i != this->Queue.end() && !found; ++i)
    {
    QueuedJob& job = *i;
    //find the first work of a given mesh type that isn't already
    //waiting for a worker
    if(job.Message.MeshIOType() == type && !job.WaitingForWorker)
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

//------------------------------------------------------------------------------
bool JobQueue::remove(const boost::uuids::uuid& id)
{
  QueuedJob* queued;
  int index = 0;
  bool found = false;
  for(index=0; index < this->Queue.size(); ++index)
    {
    queued = &this->Queue[index];
    if(queued->Id == id)
      {
      found = true;
      break;
      }
    }

  if(found)
    {
    boost::uuids::uuid id = queued->Id;

    this->QueuedIds.erase(id);
    this->Queue.erase(this->Queue.begin()+index);
    }
  return found;
}

}
}
} //namespace remus::server::internal

#endif // JOBQUEUE_H
