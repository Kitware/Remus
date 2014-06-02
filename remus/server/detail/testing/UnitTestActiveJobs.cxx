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

#ifdef _WIN32
#include <windows.h>
#endif

namespace {


void SleepForNSecs(int t)
{
#ifdef _WIN32
      Sleep(t*1000);
#else
      sleep(t);
#endif
}

remus::server::detail::SocketMonitor make_Monitor( )
{
  //make the timeouts on the poller to be 1 second for min and max, to make
  //checking for expiration far easier
  remus::common::PollingMonitor poller(1,1);
  remus::server::detail::SocketMonitor sm(poller);
  return sm;
}

//makes a random socket identity
zmq::SocketIdentity make_socketId()
{
  boost::uuids::uuid new_uid = remus::testing::UUIDGenerator();
  const std::string str_id = boost::lexical_cast<std::string>(new_uid);
  return zmq::SocketIdentity(str_id.c_str(),str_id.size());
}


void verify_add_remove_jobs()
{
  std::vector< boost::uuids::uuid > uuids_used;
  for(int i=0; i < 5; ++i)
    { uuids_used.push_back(remus::testing::UUIDGenerator()); }

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

  REMUS_ASSERT( (jobs.remove(remus::testing::UUIDGenerator()) == false));
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
  std::set< zmq::SocketIdentity > valid_workers = jobs.activeWorkers();
  REMUS_ASSERT( (valid_workers.size() == 4) );
  for(int i=1; i < 5; ++i)
    {
    zmq::SocketIdentity workerAddress = jobs.workerAddress(uuids_used[i]);
    REMUS_ASSERT( (valid_workers.count(workerAddress) == 1) );
    }
  //verify that we don't have the removed jobs
  zmq::SocketIdentity workerAddress = jobs.workerAddress(uuids_used[0]);
  REMUS_ASSERT( (valid_workers.count(workerAddress) == 0) );
  REMUS_ASSERT( (jobs.haveUUID(uuids_used[0]) == false) );
}

void verify_updating_status()
{
  std::vector< boost::uuids::uuid > uuids_used;
  for(int i=0; i < 5; ++i)
    { uuids_used.push_back(remus::testing::UUIDGenerator()); }

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
      remus::proto::JobStatus status(uuids_used[0], status_type);
      jobs.updateStatus(status);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).status() != status_type) );

      remus::proto::JobResult result(uuids_used[0]);
      jobs.updateResult(result);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == status_type) );
      REMUS_ASSERT( (jobs.result(uuids_used[0]).valid() == false) );

      remus::proto::JobResult result_with_data =
                        remus::proto::make_JobResult(uuids_used[0],"data");
      jobs.updateResult(result_with_data);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == status_type) );
      REMUS_ASSERT( (jobs.result(uuids_used[0]).valid() == true) );
      }
    else
      {
      remus::proto::JobStatus status(uuids_used[0], status_type);
      jobs.updateStatus(status);
      }

    REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == status_type) );
    for(remus::STATUS_TYPE other_status = remus::QUEUED;
        other_status != status_type;
        other_status = remus::STATUS_TYPE((int)other_status+1))
      {
      remus::proto::JobStatus status_attempt(uuids_used[0], other_status);
      jobs.updateStatus(status_attempt);
      REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == status_type) );
      }

    }

  //verify that finished is greater than expired or failed
  remus::proto::JobStatus status_failed(uuids_used[0], remus::FAILED);
  jobs.updateStatus(status_failed);
  REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == remus::FINISHED) );

  remus::proto::JobStatus status_expired(uuids_used[0], remus::EXPIRED);
  jobs.updateStatus(status_expired);
  REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == remus::FINISHED) );


  remus::proto::JobStatus status_mk1(uuids_used[1], remus::FAILED);
  jobs.updateStatus(status_mk1);
  //we know have to do it all over again, with the first stage
  //being finished, and verify it can't move out of that stage
  //expect for failed
  for(remus::STATUS_TYPE status_type = remus::QUEUED;
      status_type != remus::FAILED;
      status_type = remus::STATUS_TYPE((int)status_type+1))
    {
    remus::proto::JobStatus status(uuids_used[1], status_type);
    jobs.updateStatus(status);
    REMUS_ASSERT( (jobs.status(uuids_used[1]).status() == remus::FAILED) );
    }

  remus::proto::JobStatus status_mk2(uuids_used[2], remus::EXPIRED);
  jobs.updateStatus(status_mk2);
  //we know have to do it all over again, with the first stage
  //being failed, and verify it can't move out of that stage
  for(remus::STATUS_TYPE status_type = remus::QUEUED;
      status_type != remus::EXPIRED;
      status_type = remus::STATUS_TYPE((int)status_type+1))
    {
    remus::proto::JobStatus status(uuids_used[2], status_type);
    jobs.updateStatus(status);
    REMUS_ASSERT( (jobs.status(uuids_used[2]).status() == remus::EXPIRED) );
    }


  //verify the status of all 5 jobs.
  //0 = finished with result
  //1 = FAILED
  //2 = EXPIRED
  //3 = QUEUED
  //4 = QUEUED
  REMUS_ASSERT( (jobs.status(uuids_used[0]).status() == remus::FINISHED) );
  REMUS_ASSERT( (jobs.status(uuids_used[1]).status() == remus::FAILED) );
  REMUS_ASSERT( (jobs.status(uuids_used[2]).status() == remus::EXPIRED) );
  REMUS_ASSERT( (jobs.status(uuids_used[3]).status() == remus::QUEUED) );
  REMUS_ASSERT( (jobs.status(uuids_used[4]).status() == remus::QUEUED) );

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
  boost::uuids::uuid uuid_used = remus::testing::UUIDGenerator();
  remus::server::detail::ActiveJobs jobs;

  REMUS_ASSERT( (jobs.add(make_socketId(), uuid_used) == true) );
  REMUS_ASSERT( (jobs.haveUUID(uuid_used) == true) );
  REMUS_ASSERT( (jobs.haveResult(uuid_used) == false) );

  REMUS_ASSERT( (jobs.status(uuid_used).queued() == true) );

  remus::proto::JobStatus wjs(uuid_used, remus::proto::JobProgress(5) );
  jobs.updateStatus(wjs);
  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().value() == 5) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().message().size() == 0) );

  wjs.updateProgress( remus::proto::JobProgress(20,"random text") );
  REMUS_ASSERT( (wjs.inProgress() == true) );
  jobs.updateStatus(wjs);

  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().value() == 20) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().message() == "random text") );

  //lets go backwards in progress, this should work
  wjs.updateProgress( remus::proto::JobProgress(19,"random text v2"));
  REMUS_ASSERT( (wjs.inProgress() == true) );
  jobs.updateStatus(wjs);

  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().value() == 19) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().message() == "random text v2" ) );

  //lets go backwards in progress and remove the text, this should work
  wjs.updateProgress( remus::proto::JobProgress(2));
  REMUS_ASSERT( (wjs.inProgress() == true) );
  jobs.updateStatus(wjs);

  REMUS_ASSERT( (jobs.status(uuid_used).inProgress() == true) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().value() == 2) );
  REMUS_ASSERT( (jobs.status(uuid_used).progress().message() == std::string() ) );
}

void verify_refresh_jobs()
{
  //we need to verify that the refresh and expired functions work properly
  typedef remus::server::detail::SocketMonitor MonitorType;
  MonitorType monitor = make_Monitor( );

  std::vector< boost::uuids::uuid > uuids_used;
  std::vector< zmq::SocketIdentity > socketIds_used;

  for(int i=0; i < 5; ++i)
    {
    uuids_used.push_back( remus::testing::UUIDGenerator() );

    const zmq::SocketIdentity sId =  make_socketId();
    monitor.refresh(sId);
    socketIds_used.push_back( sId );
    }

  remus::server::detail::ActiveJobs jobs;

  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.add(socketIds_used[i], uuids_used[i]) == true) ); }

  jobs.markExpiredJobs( monitor );
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).status() == remus::QUEUED) ); }

  SleepForNSecs(1);

  //even after 1 second we aren't expired
  jobs.markExpiredJobs( monitor );
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).status() == remus::QUEUED) ); }

  for(int i=0; i < 2; ++i)
    {
    SleepForNSecs(1);
    for(int j=0; j < 5; ++j)
      { monitor.refresh(socketIds_used[j]); }
    }

  //even after 2 more seconds we aren't expired, since we sent
  //refresh / heartbeats
  jobs.markExpiredJobs( monitor );
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).status() == remus::QUEUED) ); }


  //verify that jobs marked as finished can't be marked as expired
  boost::uuids::uuid finished_job_uuid = remus::testing::UUIDGenerator();
  remus::proto::JobResult result_with_data =
                        remus::proto::make_JobResult(finished_job_uuid,"data");
  const zmq::SocketIdentity finishedJobSocketId =  make_socketId();
  jobs.add(finishedJobSocketId, finished_job_uuid);
  jobs.updateResult(result_with_data);

  monitor.refresh( finishedJobSocketId );
  jobs.markExpiredJobs( monitor );
  REMUS_ASSERT( (jobs.status(finished_job_uuid).status() == remus::FINISHED) );
}


void verify_expire_jobs()
{
  //we need to verify that the refresh and expired functions work properly
  typedef remus::server::detail::SocketMonitor MonitorType;
  MonitorType monitor = make_Monitor( );

  std::vector< boost::uuids::uuid > uuids_used;
  std::vector< zmq::SocketIdentity > socketIds_used;

  for(int i=0; i < 5; ++i)
    {
    uuids_used.push_back( remus::testing::UUIDGenerator() );

    const zmq::SocketIdentity sId =  make_socketId();
    monitor.refresh(sId);
    socketIds_used.push_back( sId );
    }

  remus::server::detail::ActiveJobs jobs;

  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.add(socketIds_used[i], uuids_used[i]) == true) ); }

  jobs.markExpiredJobs( monitor );
  for(int i=0; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).status() == remus::QUEUED) ); }

  //wait for 3 second which will cause the jobs 3 and 4 to fail
  //while jobs 0, 1 and 2 will be refreshed and will be valid
  for(int i=0; i < 3; ++i)
    {
    SleepForNSecs(1);
    for(int j=0; j < 3; ++j)
      { monitor.refresh(socketIds_used[j]); }
    }

  jobs.markExpiredJobs( monitor );
  for(int i=0; i < 3; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).status() == remus::QUEUED) ); }
  for(int i=3; i < 5; ++i)
    { REMUS_ASSERT( (jobs.status(uuids_used[i]).status() == remus::EXPIRED) ); }

}

} //namespace

int UnitTestActiveJobs(int, char *[])
{
  verify_add_remove_jobs();

  verify_updating_status();

  verify_updating_progress();

  verify_refresh_jobs();

  verify_expire_jobs();

  return 0;
}
