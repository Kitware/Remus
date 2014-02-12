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

#include <remus/common/ContentTypes.h>
#include <remus/server/detail/uuidHelper.h>
#include <remus/testing/Testing.h>

#include <boost/uuid/random_generator.hpp>

namespace {

using namespace remus::common;
using namespace remus::meshtypes;

const MeshIOType worker_type1D( (Edges()), (Mesh1D()) );
const MeshIOType worker_type2D( (Edges()), (Mesh2D()) );
const MeshIOType worker_type3D( (Edges()), (Mesh3D()) );

boost::uuids::random_generator generator;


//makes a random socket identity
boost::uuids::uuid make_id()
{
  return generator();
}

//make a random JobSubmission
template<typename T, typename U>
remus::proto::JobSubmission make_jobSubmission(const T& t, const U& u)
{
  ContentSource::Type stype(ContentSource::File);
  ContentFormat::Type ftype(ContentFormat::USER);
  MeshIOType mtype(t,u);
  std::string name = remus::testing::AsciiStringGenerator(25);
  std::string tag;
  ConditionalStorage storage;

  std::stringstream buffer;

  buffer << stype << std::endl;
  buffer << ftype << std::endl;
  buffer << mtype << std::endl;

  buffer << name.size() << std::endl;
  remus::internal::writeString( buffer, name );

  buffer << tag.size() << std::endl;
  remus::internal::writeString( buffer,  tag);


  buffer << storage.size() << std::endl;
  remus::internal::writeString( buffer,
                                storage.get(),
                                storage.size() );


  remus::proto::JobRequirements requirements;
  buffer >> requirements;
  remus::proto::JobSubmission submission( requirements );
  return submission;
}



void verify_add_remove_jobs()
{
  remus::server::detail::JobQueue queue;

  std::vector< boost::uuids::uuid > uuids_used;
  for(int i=0; i < 7; ++i) { uuids_used.push_back(make_id()); }

  remus::proto::JobSubmission JobSubmission = make_jobSubmission(Edges(),Mesh3D());

  REMUS_ASSERT( (queue.addJob( uuids_used[0], JobSubmission ) == true) );

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );

  //verify we can't use the same id too
  REMUS_ASSERT( (queue.addJob( uuids_used[0], JobSubmission ) ==false) );

  //add a ton of jobs and make sure we can remove all them
  queue.addJob( uuids_used[1], JobSubmission );
  queue.addJob( uuids_used[2], JobSubmission );
  queue.addJob( uuids_used[3], JobSubmission );

  queue.addJob( uuids_used[4], make_jobSubmission(Edges(),Mesh2D()) );
  queue.addJob( uuids_used[5], make_jobSubmission(Edges(),Mesh2D()) );
  queue.addJob( uuids_used[6], make_jobSubmission(Edges(),Mesh2D()) );

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 2) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );
  REMUS_ASSERT( (queue.haveUUID(uuids_used[0]) == true) );
  REMUS_ASSERT( (queue.haveUUID(make_id()) == false) );

  //lets try to remove all the bu the first job
  for(int i=1; i < 7; ++i)
    {
    REMUS_ASSERT( (queue.remove(uuids_used[i]) == true) );
    REMUS_ASSERT( (queue.haveUUID(uuids_used[i]) == false) );
    }

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );

  //clear the queue
  queue.clear();

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 0) );

  REMUS_ASSERT( (queue.numJobsWaitingForWokers() == 0) );
  REMUS_ASSERT( (queue.numJobsJustQueued() == 0) );
}

void verify_dispatch_jobs()
{
  remus::server::detail::JobQueue queue;

  const boost::uuids::uuid j_id = make_id();
  remus::proto::JobSubmission JobSubmission = make_jobSubmission(Edges(),Mesh3D());

  queue.addJob( j_id, JobSubmission );

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );

  //add a ton of jobs and make sure we can remove all them
  queue.addJob( make_id(), JobSubmission );
  queue.addJob( make_id(), JobSubmission );
  queue.addJob( make_id(), JobSubmission );

  queue.addJob( make_id(), make_jobSubmission(Edges(),Mesh2D()) );
  queue.addJob( make_id(), make_jobSubmission(Edges(),Mesh2D()) );
  queue.addJob( make_id(), make_jobSubmission(Edges(),Mesh2D()) );

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 2) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );
  REMUS_ASSERT( (queue.haveUUID(j_id) == true) );
  REMUS_ASSERT( (queue.haveUUID(make_id()) == false) );


  //lets move some jobs over to being dispatched
  REMUS_ASSERT( (queue.workerDispatched(worker_type1D) == false) );
  REMUS_ASSERT( (queue.workerDispatched(worker_type2D) == true) );

  //verify the state of both queues
  REMUS_ASSERT( (queue.queuedJobTypes().size() == 2) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type2D) == 1) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type3D) == 0) );

  REMUS_ASSERT( (queue.numJobsWaitingForWokers() == 1) );
  REMUS_ASSERT( (queue.numJobsJustQueued() == 6) );


  //now lets move a couple more jobs to be waiting for a worker
  REMUS_ASSERT( (queue.workerDispatched(worker_type2D) == true) );
  REMUS_ASSERT( (queue.workerDispatched(worker_type3D) == true) );
  REMUS_ASSERT( (queue.workerDispatched(worker_type3D) == true) );

  REMUS_ASSERT( (queue.numJobsWaitingForWokers() == 4) );
  REMUS_ASSERT( (queue.numJobsJustQueued() == 3) );

  //verify the state of both queues
  REMUS_ASSERT( (queue.queuedJobTypes().size() == 2) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 1) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 1) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type2D) == 1) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type3D) == 1) );

  REMUS_ASSERT( (queue.takeJob(worker_type1D).valid() == false) );

  remus::worker::Job job_2d_1 = queue.takeJob(worker_type2D);
  REMUS_ASSERT( (job_2d_1.valid() == true) );
  REMUS_ASSERT( (queue.haveUUID(job_2d_1.id()) == false) );

  remus::worker::Job job_2d_2 = queue.takeJob(worker_type2D);
  REMUS_ASSERT( (job_2d_2.valid() == true) );
  REMUS_ASSERT( (job_2d_2.id() != job_2d_1.id()) );
  REMUS_ASSERT( (queue.haveUUID(job_2d_2.id()) == false) );

  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type3D) == 1) );

  REMUS_ASSERT( (queue.takeJob(worker_type3D).valid() == true) );
  REMUS_ASSERT( (queue.takeJob(worker_type3D).valid() == true) );
  REMUS_ASSERT( (queue.takeJob(worker_type3D).valid() == true) );

  REMUS_ASSERT( (queue.numJobsWaitingForWokers() == 0) );
  REMUS_ASSERT( (queue.numJobsJustQueued() == 2) );

  //take job will get from the just queued list once the waiting list
  //is empty
  REMUS_ASSERT( (queue.takeJob(worker_type1D).valid() == false) );
  REMUS_ASSERT( (queue.takeJob(worker_type2D).valid() == true) );
  REMUS_ASSERT( (queue.takeJob(worker_type3D).valid() == true) );

  REMUS_ASSERT( (queue.numJobsWaitingForWokers() == 0) );
  REMUS_ASSERT( (queue.numJobsJustQueued() == 0) );

  REMUS_ASSERT( (queue.queuedJobTypes().size() == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.queuedJobTypes().count(worker_type3D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type1D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type2D) == 0) );
  REMUS_ASSERT( (queue.waitingForWorkerTypes().count(worker_type3D) == 0) );
}

} //namespace

int UnitTestJobQueue(int, char *[])
{

  verify_add_remove_jobs();

  verify_dispatch_jobs();


  return 0;
}
