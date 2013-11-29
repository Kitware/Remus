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

#include <remus/client/JobRequest.h>
#include <remus/common/Message.h>
#include <remus/server/internal/uuidHelper.h>
#include <remus/worker/Job.h>

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <algorithm>
#include <set>
#include <vector>

namespace remus{
namespace server{
namespace internal{

//A FIFO queue. each mesh type has its own queue
//where we keep jobs. The uuid for each job
//must be unique.

class JobQueue
{
public:
  JobQueue():
    QueuedJobs(),
    QueuedJobsForWorkers(),
    QueuedIds()
    {}

  //Convert a Message and UUID into a WorkerMessage.
  //will return false if the uuid is already queued
  bool addJob( const boost::uuids::uuid& id,
               const remus::common::Message& message);

  //Removes a job from the queue of the given mesh type.
  //Return it as a worker Job. We prioritize jobs waiting for
  //workers, and than take jobs that are just queued.
  remus::worker::Job takeJob(remus::common::MeshIOType type);

  //returns the types of jobs that are waiting for a worker
  std::set<remus::common::MeshIOType> waitingForWorkerTypes() const;

  //returns the types of jobs that are queued and aren't waiting for a worker
  std::set<remus::common::MeshIOType> queuedJobTypes() const;

  //return the number of jobs waiting for workers
  unsigned int numJobsWaitingForWokers() const
    { return QueuedJobsForWorkers.size(); }

  //return the number of jobs queued but not waiting for a worker
  unsigned int numJobsJustQueued() const
    { return QueuedJobs.size(); }

  //marks the first job with the given type as having
  //a worker dispatched for it.
  bool workerDispatched(remus::common::MeshIOType type);

  //Returns true if we contain the UUID
  bool haveUUID(const boost::uuids::uuid& id) const;

  //Returns true if we can remove a job with a give uuid
  bool remove(const boost::uuids::uuid& id);

  //Removes all queued and waiting for worker jobs.
  void clear();

private:
  struct QueuedJob
  {
    QueuedJob(const boost::uuids::uuid& id,
              const remus::common::Message& message):
              Id(id),
              Message(message),
              WorkerDispatchTime(boost::posix_time::second_clock::local_time())
              {}

    boost::uuids::uuid Id;
    remus::common::Message Message;

    //information on when the job was marked as scheduled
    boost::posix_time::ptime WorkerDispatchTime;

  };

  struct CountTypes
  {
    void operator()( const QueuedJob& job )
      { types.insert(job.Message.MeshIOType()); }
    std::set<remus::common::MeshIOType> types;
  };

  struct JobIdMatches
  {
    JobIdMatches(boost::uuids::uuid id):
    UUID(id) {}

    bool operator()(const QueuedJob& job) const
      { return job.Id == UUID; }

    boost::uuids::uuid UUID;
  };

  struct JobTypeMatches
  {
    JobTypeMatches(remus::common::MeshIOType t):
    type(t) {}

    bool operator()(const QueuedJob& job) const
      { return type == job.Message.MeshIOType(); }

    remus::common::MeshIOType type;
  };

  //job queue info
  std::vector<QueuedJob> QueuedJobs;
  std::vector<QueuedJob> QueuedJobsForWorkers;
  std::set<boost::uuids::uuid> QueuedIds;

  //make copying not possible
  JobQueue (const JobQueue&);
  void operator = (const JobQueue&);
};


//------------------------------------------------------------------------------
bool JobQueue::addJob(const boost::uuids::uuid &id,
                      const remus::common::Message& message)
{
  //only add the message as a job if the uuid hasn't been used already
  const bool can_add = QueuedIds.count(id) == 0;
  if(can_add)
    {
    this->QueuedJobs.push_back(QueuedJob(id,message));
    this->QueuedIds.insert(id);
    }
  return can_add;
}

//------------------------------------------------------------------------------
remus::worker::Job JobQueue::takeJob(remus::common::MeshIOType type)
{
  std::vector<QueuedJob>* searched_vector = &this->QueuedJobsForWorkers;
  typedef std::vector<QueuedJob>::iterator iter;

  JobTypeMatches pred(type);
  iter item = std::find_if(searched_vector->begin(), searched_vector->end(),
                           pred);
  if(item == searched_vector->end())
    {
    searched_vector = &this->QueuedJobs;
    item = std::find_if(searched_vector->begin(), searched_vector->end(),
                        pred);
    if(item == searched_vector->end())
      {
      //return an invalid job
      return remus::worker::Job();
      }
    }

  //we need to copy the id and the contents of item now into a job
  //request, if we use item after the remove_if it is invalid as
  //remove_if moves the vector items around making what item
  //is pointing too change

  //todo this needs to be easier to convert an encoded job request
  //into the job that is being sent to the worker, without having to copy
  // the data so many times.
  const remus::client::JobRequest request(remus::client::to_JobRequest(item->Message.data(),
                                          item->Message.dataSize()));
  remus::worker::Job job(item->Id,type,request.jobInfo());

  //again don't use item after the remove_if the iterator is invalid

  JobIdMatches id_pred(job.id());

  iter new_end = std::remove_if(searched_vector->begin(),
                                searched_vector->end(),
                                id_pred);
  searched_vector->erase(new_end,searched_vector->end());
  this->QueuedIds.erase(job.id());

  // std::cout << "JobQueue::takeJob " << job.id() << std::endl;

  return job;
}

//------------------------------------------------------------------------------
std::set<remus::common::MeshIOType> JobQueue::waitingForWorkerTypes() const
{
  JobQueue::CountTypes result =
         std::for_each(this->QueuedJobsForWorkers.begin(),
                       this->QueuedJobsForWorkers.end(),
                       JobQueue::CountTypes());
  return result.types;
}

//------------------------------------------------------------------------------
std::set<remus::common::MeshIOType> JobQueue::queuedJobTypes() const
{
  JobQueue::CountTypes result =
          std::for_each(this->QueuedJobs.begin(),
                        this->QueuedJobs.end(),
                        JobQueue::CountTypes());
  return result.types;
}

//------------------------------------------------------------------------------
bool JobQueue::workerDispatched(remus::common::MeshIOType type)
{
  typedef std::vector<QueuedJob>::iterator iter;
  JobTypeMatches pred(type);

  iter item = std::find_if(this->QueuedJobs.begin(),
                           this->QueuedJobs.end(), pred);
  const bool found = this->QueuedJobs.end() != item;
  if(found)
    {
    item->WorkerDispatchTime = boost::posix_time::second_clock::local_time();
    this->QueuedJobsForWorkers.push_back(*item);
    this->QueuedJobs.erase(item);
    }
  return found;
}

//------------------------------------------------------------------------------
bool JobQueue::haveUUID(const boost::uuids::uuid &id) const
{
  return this->QueuedIds.count(id) == 1;
}

//------------------------------------------------------------------------------
bool JobQueue::remove(const boost::uuids::uuid& id)
{
  typedef std::vector<QueuedJob>::iterator iter;
  JobIdMatches pred(id);

  iter new_end = std::remove_if(this->QueuedJobs.begin(),
                                this->QueuedJobs.end(),
                                pred);
  this->QueuedJobs.erase(new_end,this->QueuedJobs.end());

  new_end = std::remove_if(this->QueuedJobsForWorkers.begin(),
                           this->QueuedJobsForWorkers.end(),
                           pred);
  this->QueuedJobsForWorkers.erase(new_end,this->QueuedJobsForWorkers.end());

  return this->QueuedIds.erase(id)==1;
}

//------------------------------------------------------------------------------
void JobQueue::clear()
{
  this->QueuedIds.clear();
  this->QueuedJobs.clear();
  this->QueuedJobs.clear();
}

}
}
} //namespace remus::server::internal

#endif // JOBQUEUE_H
