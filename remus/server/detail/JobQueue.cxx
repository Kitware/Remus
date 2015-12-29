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

#include <remus/server/detail/JobQueue.h>
#include <iostream>

namespace remus{
namespace server{
namespace detail{

//------------------------------------------------------------------------------
bool JobQueue::addJob(const boost::uuids::uuid &id,
                      const remus::proto::JobSubmission& submission)
{
  //only add the message as a job if the uuid hasn't been used already
  const bool can_add = QueuedIds.count(id) == 0;
  if(can_add)
    {
    QueuedJob newQueuedJob(id,submission);
    this->QueuedJobs.insert(
          std::lower_bound( this->QueuedJobs.begin(), this->QueuedJobs.end(),
                            newQueuedJob ),
          newQueuedJob);
    this->QueuedIds.insert(id);
    this->CachedQueuedJobRequirements.insert( submission.requirements() );
    }
  return can_add;
}

//------------------------------------------------------------------------------
remus::worker::Job JobQueue::takeJob(const remus::proto::JobRequirements& reqs)
{
  std::vector<QueuedJob>* searched_vector = &this->JobsWaitingForWorker;
  typedef std::vector<QueuedJob>::iterator iter;

  JobTypeMatches pred(reqs);
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

    //the job is from the queue, so invalidate the cache. This is overaggressive
    //but to fix it we would need to track the number of jobs that each requirement
    //maps too
    this->CachedQueuedJobRequirements.clear();
    }

  //we need to copy the id and the contents of item now into a job
  //submission, if we use item after the remove_if it is invalid as
  //remove_if moves the vector items around making what item
  //is pointing too change
  remus::worker::Job job(item->Id,item->Submission);

  //again don't use item after the remove_if the iterator is invalid
  JobIdMatches id_pred(job.id());

  iter new_end = std::remove_if(searched_vector->begin(),
                                searched_vector->end(),
                                id_pred);
  searched_vector->erase(new_end,searched_vector->end());

  this->QueuedIds.erase(job.id());

  //clear the queued job cache

  return job;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet JobQueue::waitingJobRequirements() const
{
  remus::proto::JobRequirementsSet result;
  for(std::vector<QueuedJob>::const_iterator i= this->JobsWaitingForWorker.begin();
      i != this->JobsWaitingForWorker.end();
      ++i)
    {
    result.insert(i->Submission.requirements());
    }
  return result;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet JobQueue::queuedJobRequirements()
{
  if(this->CachedQueuedJobRequirements.size() == 0)
    {
    for(std::vector<QueuedJob>::const_iterator i= this->QueuedJobs.begin();
      i != this->QueuedJobs.end();
      ++i)
      {
      this->CachedQueuedJobRequirements.insert(i->Submission.requirements());
      }
    }

  return CachedQueuedJobRequirements;
}

//------------------------------------------------------------------------------
bool JobQueue::workerDispatched(const remus::proto::JobRequirements& reqs)
{
  typedef std::vector<QueuedJob>::iterator iter;
  JobTypeMatches pred(reqs);

  iter item = std::find_if(this->QueuedJobs.begin(),
                           this->QueuedJobs.end(), pred);
  const bool found = this->QueuedJobs.end() != item;
  if(found)
    {
    this->JobsWaitingForWorker.push_back(*item);
    this->QueuedJobs.erase(item);
    this->CachedQueuedJobRequirements.clear();
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

  bool id_found = false;

  iter new_end = std::remove_if(this->QueuedJobs.begin(),
                                this->QueuedJobs.end(),
                                pred);

  id_found = (new_end != this->QueuedJobs.end());
  if( id_found )
    {
    this->QueuedJobs.erase(new_end,this->QueuedJobs.end());
    this->CachedQueuedJobRequirements.clear();
    }
  else
    {
    new_end = std::remove_if(this->JobsWaitingForWorker.begin(),
                            this->JobsWaitingForWorker.end(),
                            pred);

    id_found = (new_end != this->JobsWaitingForWorker.end());

    if( id_found )
      {
      this->JobsWaitingForWorker.erase(new_end,this->JobsWaitingForWorker.end());
      }
    }
  return this->QueuedIds.erase(id)==1;
}

//------------------------------------------------------------------------------
void JobQueue::clear()
{
  this->QueuedIds.clear();
  this->QueuedJobs.clear();
  this->QueuedJobs.clear();
  this->CachedQueuedJobRequirements.clear();
}

}
}
} //namespace remus::server::detail
