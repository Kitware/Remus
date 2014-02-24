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
    }
  return can_add;
}

//------------------------------------------------------------------------------
remus::worker::Job JobQueue::takeJob(const remus::proto::JobRequirements& reqs)
{
  std::vector<QueuedJob>* searched_vector = &this->QueuedJobsForWorkers;
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

  // std::cout << "JobQueue::takeJob " << job.id() << std::endl;

  return job;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet JobQueue::waitingJobRequirements() const
{
  JobQueue::CollectRequirements result =
         std::for_each(this->QueuedJobsForWorkers.begin(),
                       this->QueuedJobsForWorkers.end(),
                       JobQueue::CollectRequirements());
  return result.types;
}

//------------------------------------------------------------------------------
remus::proto::JobRequirementsSet JobQueue::queuedJobRequirements() const
{
  JobQueue::CollectRequirements result =
          std::for_each(this->QueuedJobs.begin(),
                        this->QueuedJobs.end(),
                        JobQueue::CollectRequirements());
  return result.types;
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
} //namespace remus::server::detail
