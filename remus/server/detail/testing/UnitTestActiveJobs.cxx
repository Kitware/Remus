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


#include <remus/testing/Testing.h>
#include <remus/server/detail/ActiveJobs.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/random_generator.hpp>


namespace {

boost::uuids::random_generator generator;

//makes a random uuid
boost::uuids::uuid make_id()
{
  return generator();
}

//makes a random socket identity
zmq::socketIdentity make_socketId()
{
  boost::uuids::uuid new_uid = make_id();
  const std::string str_id = boost::lexical_cast<std::string>(new_uid);
  return zmq::socketIdentity(str_id.c_str(),str_id.size());
}


void verify_add_remove_jobs()
{
  std::vector< boost::uuids::uuid > uuids_used;
  for(int i=0; i < 5; ++i) { uuids_used.push_back(make_id()); }

  remus::server::detail::ActiveJobs jobs;

  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.add(make_socketId(), uuids_used[i]) == true) ); }

  for(int i=0; i < 5; ++i)
    {
    REMUS_ASSERT( (jobs.add(make_socketId(), uuids_used[i]) == false) );
    REMUS_ASSERT( (jobs.haveUUID(uuids_used[i]) == true) );
    REMUS_ASSERT( (jobs.haveResult(uuids_used[i]) == false) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).good() == true) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).queued() == true) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).inProgress() == false) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).failed() == false) );
    }

  REMUS_ASSERT( (jobs.remove(make_id()) == false));
  REMUS_ASSERT( (jobs.remove(uuids_used[0]) == true));
  REMUS_ASSERT( (jobs.remove(uuids_used[0]) == false));

  //verify removing the first job didn't change any of the other jobs
  for(int i=1; i < 5; ++i)
    {
    REMUS_ASSERT( (jobs.add(make_socketId(), uuids_used[i]) == false) );
    REMUS_ASSERT( (jobs.haveUUID(uuids_used[i]) == true) );
    REMUS_ASSERT( (jobs.haveResult(uuids_used[i]) == false) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).good() == true) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).queued() == true) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).inProgress() == false) );
    REMUS_ASSERT( (jobs.status(uuids_used[i]).failed() == false) );
    }

  //verify the contents of the set returned by activeWorkers is correct
  std::set< zmq::socketIdentity > valid_workers = jobs.activeWorkers();
  REMUS_ASSERT( (valid_workers.size() == 4) );
  for(int i=1; i < 5; ++i)
    {
    zmq::socketIdentity workerAddress = jobs.workerAddress(uuids_used[i]);
    REMUS_ASSERT( (valid_workers.count(workerAddress) == 1) );
    }
  //verify that we don't have the removed jobs
  zmq::socketIdentity workerAddress = jobs.workerAddress(uuids_used[0]);
  REMUS_ASSERT( (valid_workers.count(workerAddress) == 0) );
  REMUS_ASSERT( (jobs.haveUUID(uuids_used[0]) == false) );
}

void verify_updating_status()
{
  std::vector< boost::uuids::uuid > uuids_used;
  for(int i=0; i < 5; ++i) { uuids_used.push_back(make_id()); }

  remus::server::detail::ActiveJobs jobs;

  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.add(make_socketId(), uuids_used[i]) == true) ); }

  //Active Jobs is the class with the most 'conditions'
  //we are just testing QUEUED and IN_PROGRESS interactions
  for(remus::STATUS_TYPE status_type = remus::QUEUED;
      status_type != remus::FAILED;
      status_type = remus::STATUS_TYPE((int)status_type+1))
    {

    if(status_type == remus::FINISHED)
      {
      //to set the finished state you need to actually call updateResult,
      //not updateStatus
      remus::worker::JobStatus status(uuids_used[0], status_type);
      jobs.updateStatus(status);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).Status != status_type) );

      remus::worker::JobResult result(uuids_used[0]);
      jobs.updateResult(result);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == status_type) );
      REMUS_ASSERT( (jobs.result(uuids_used[0]).valid() == false) );

      remus::worker::JobResult result_with_data(uuids_used[0],"data");
      jobs.updateResult(result_with_data);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == status_type) );
      REMUS_ASSERT( (jobs.result(uuids_used[0]).valid() == true) );
      }
    else
      {
      remus::worker::JobStatus status(uuids_used[0], status_type);
      jobs.updateStatus(status);
      }

    REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == status_type) );
    for(remus::STATUS_TYPE other_status = remus::QUEUED;
        other_status != status_type;
        other_status = remus::STATUS_TYPE((int)other_status+1))
      {
      remus::worker::JobStatus status_attempt(uuids_used[0], other_status);
      jobs.updateStatus(status_attempt);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == status_type) );
      }

    }

  //verify that finished is greater than expired or failed
  remus::worker::JobStatus status_failed(uuids_used[0], remus::FAILED);
  jobs.updateStatus(status_failed);
  REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == remus::FINISHED) );

  remus::worker::JobStatus status_expired(uuids_used[0], remus::EXPIRED);
  jobs.updateStatus(status_expired);
  REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == remus::FINISHED) );


  remus::worker::JobStatus status_mk1(uuids_used[1], remus::FAILED);
  jobs.updateStatus(status_mk1);
  //we know have to do it all over again, with the first stage
  //being finished, and verify it can't move out of that stage
  //expect for failed
  for(remus::STATUS_TYPE status_type = remus::QUEUED;
      status_type != remus::FAILED;
      status_type = remus::STATUS_TYPE((int)status_type+1))
    {
    remus::worker::JobStatus status(uuids_used[1], status_type);
    jobs.updateStatus(status);
    REMUS_ASSERT( (jobs.status(uuids_used[1]).Status == remus::FAILED) );
    }

  remus::worker::JobStatus status_mk2(uuids_used[2], remus::EXPIRED);
  jobs.updateStatus(status_mk2);
  //we know have to do it all over again, with the first stage
  //being failed, and verify it can't move out of that stage
  for(remus::STATUS_TYPE status_type = remus::QUEUED;
      status_type != remus::EXPIRED;
      status_type = remus::STATUS_TYPE((int)status_type+1))
    {
    remus::worker::JobStatus status(uuids_used[2], status_type);
    jobs.updateStatus(status);
    REMUS_ASSERT( (jobs.status(uuids_used[2]).Status == remus::EXPIRED) );
    }


  //verify the status of all 5 jobs.
  //0 = finished with result
  //1 = FAILED
  //2 = EXPIRED
  //3 = QUEUED
  //4 = QUEUED
  REMUS_ASSERT( (jobs.status(uuids_used[0]).Status == remus::FINISHED) );
  REMUS_ASSERT( (jobs.status(uuids_used[1]).Status == remus::FAILED) );
  REMUS_ASSERT( (jobs.status(uuids_used[2]).Status == remus::EXPIRED) );
  REMUS_ASSERT( (jobs.status(uuids_used[3]).Status == remus::QUEUED) );
  REMUS_ASSERT( (jobs.status(uuids_used[4]).Status == remus::QUEUED) );

  //check for results
  REMUS_ASSERT( (jobs.haveResult(uuids_used[0]) == true) );
  REMUS_ASSERT( (jobs.haveResult(uuids_used[1]) == false) );
  REMUS_ASSERT( (jobs.haveResult(uuids_used[2]) == false) );
  REMUS_ASSERT( (jobs.haveResult(uuids_used[3]) == false) );
  REMUS_ASSERT( (jobs.haveResult(uuids_used[4]) == false) );

  REMUS_ASSERT( (jobs.activeWorkers().size() == 5) );

}


void verify_updating_progress()
{
  boost::uuids::uuid uuid_used = make_id();
  remus::server::detail::ActiveJobs jobs;

  REMUS_ASSERT( (jobs.add(make_socketId(), uuid_used) == true) );
  REMUS_ASSERT( (jobs.haveUUID(uuid_used) == true) );
  REMUS_ASSERT( (jobs.haveResult(uuid_used) == false) );

  REMUS_ASSERT( (jobs.status(uuid_used).queued() == true) );

  remus::worker::JobStatus wjs(uuid_used, remus::worker::JobProgress(5) );
  jobs.updateStatus(wjs);
  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).Progress.value() == 5) );
  REMUS_ASSERT( (jobs.status(uuid_used).Progress.message().size() == 0) );

  wjs.Progress.setValue(20);
  wjs.Progress.setMessage("random text");
  jobs.updateStatus(wjs);

  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).Progress.value() == 20) );
  REMUS_ASSERT( (jobs.status(uuid_used).Progress.message() == "random text") );

  //lets go backwards in progress, this should work
  wjs.Progress.setValue(19);
  wjs.Progress.setMessage(std::string()); //clear the string
  jobs.updateStatus(wjs);

  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).Progress.value() == 19) );
  REMUS_ASSERT( (jobs.status(uuid_used).Progress.message() == std::string() ) );

}

void verify_refresh_jobs()
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  //we need to verify that the refresh and expired functions work properly
  const ptime long_ago = second_clock::local_time() -  days(10);
  const ptime future = second_clock::local_time() +  days(10);
  const ptime current = second_clock::local_time();

  std::vector< boost::uuids::uuid > uuids_used;
  for(int i=0; i < 5; ++i) { uuids_used.push_back(make_id()); }

  remus::server::detail::ActiveJobs jobs;

  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.add(make_socketId(), uuids_used[i]) == true) ); }

  jobs.markExpiredJobs( long_ago );
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).Status == remus::QUEUED) ); }

  jobs.markExpiredJobs( current);
  for(int i=0; i < 5; ++i)
    {  REMUS_ASSERT( (jobs.status(uuids_used[i]).Status == remus::QUEUED) ); }

  jobs.markExpiredJobs( future);
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).Status == remus::EXPIRED) ); }

  jobs.markExpiredJobs( long_ago);
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).Status == remus::EXPIRED) ); }

  //verify that jobs marked as finished can't be marked as expired
  boost::uuids::uuid finished_job_uuid = make_id();
  remus::worker::JobResult result_with_data(finished_job_uuid,"data");
  jobs.add(make_socketId(), finished_job_uuid);
  jobs.updateResult(result_with_data);
  jobs.markExpiredJobs( future );
  REMUS_ASSERT( (jobs.status(finished_job_uuid).Status == remus::FINISHED) );

}

} //namespace

int UnitTestActiveJobs(int, char *[])
{
  verify_add_remove_jobs();

  verify_updating_status();

  verify_updating_progress();

  verify_refresh_jobs();

  return 0;
}
