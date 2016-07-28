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

#ifndef remus_server_detail_JobQueue_h
#define remus_server_detail_JobQueue_h

#include <remus/proto/JobSubmission.h>
#include <remus/proto/Message.h>

#include <remus/server/detail/uuidHelper.h>

#include <remus/worker/Job.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <algorithm>
#include <set>
#include <vector>

namespace remus{
namespace server{
namespace detail{

//A job id based queue. When jobs are added they are inserted based on
//the uuid of the job. We use the uuid as the priority of the queue to
//help alleviate the issue of a single client submitting many jobs and
//stopping any other client from getting a job finished in a reasonable
//amount of time.
class JobQueue
{
public:
  JobQueue():
    QueuedJobs(),
    JobsWaitingForWorker(),
    QueuedIds()
  {}

  //Convert a Message and UUID into a WorkerMessage.
  //will return false if the uuid is already queued
  bool addJob( const boost::uuids::uuid& id,
               const remus::proto::JobSubmission& submission);

  //Removes a job from the queue of the given mesh type.
  //Return it as a worker Job. We prioritize jobs waiting for
  //workers, and than take jobs that are just queued.
  remus::worker::Job takeJob(const remus::proto::JobRequirements& reqs);

  //returns the types of jobs that are waiting for a worker
  remus::proto::JobRequirementsSet waitingJobRequirements() const;

  //returns the types of jobs that are queued and aren't waiting for a worker
  remus::proto::JobRequirementsSet queuedJobRequirements();

  //return the number of jobs waiting for workers
  std::size_t numJobsWaitingForWorkers() const
    { return JobsWaitingForWorker.size(); }

  //return the number of jobs queued but not waiting for a worker
  std::size_t numJobsJustQueued() const
    { return QueuedJobs.size(); }

  //marks the first job with the given type as having
  //a worker dispatched for it.
  bool workerDispatched(const remus::proto::JobRequirements& reqs);

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
              const remus::proto::JobSubmission& submission):
              Id(id),
              Submission(submission)
              {}

    boost::uuids::uuid Id;
    remus::proto::JobSubmission Submission;

    bool operator<(const QueuedJob& other) const
      { return this->Id < other.Id; }

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
    JobTypeMatches(const remus::proto::JobRequirements& r):
    Reqs(r) {}

    bool operator()(const QueuedJob& job) const
      { return Reqs == job.Submission.requirements(); }

    const remus::proto::JobRequirements& Reqs;
  };

  //job queue info, sorted by id when items are added so that we can get
  //a really rough load balancing when we have multiple clients. This
  //way jobs that come up for new workers are roughly round robin when
  //the number of jobs per client is similar.
  std::vector<QueuedJob> QueuedJobs;

  //kept in the order that the jobs are added since this is a real queue
  //we want the priority of queued jobs that have a working incoming to
  //match the dispatch order
  std::vector<QueuedJob> JobsWaitingForWorker;

  std::set<boost::uuids::uuid> QueuedIds;
  std::set<remus::proto::JobRequirements> CachedQueuedJobRequirements;

  //make copying not possible
  JobQueue (const JobQueue&);
  void operator = (const JobQueue&);
};

}
}
}

#endif
