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
#include <remus/server/detail/WorkerPool.h>

#include <remus/common/SleepFor.h>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/server/detail/uuidHelper.h>

#include <remus/testing/Testing.h>

#include <vector>

namespace {

using namespace remus::common;
using namespace remus::meshtypes;

const remus::proto::JobRequirements worker_type2D(ContentFormat::User,
                                                  MeshIOType(Edges(),Mesh2D()),
                                                  "", "" );
const remus::proto::JobRequirements worker_type3D(ContentFormat::User,
                                                  MeshIOType(Edges(),Mesh3D()),
                                                  "", "" );

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


void verify_has_workers()
{
  //verify that we start with zero workers
  remus::server::detail::WorkerPool pool;
  REMUS_ASSERT( (pool.allWorkers().size() == 0) );

  //verify that if we add a worker we only have 1 worker,
  //and we have no workers ready for work
  zmq::SocketIdentity worker1_id = make_socketId();
  pool.addWorker(worker1_id, worker_type2D);
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type3D) == false) );
  REMUS_ASSERT( (pool.allWorkers().count(worker1_id) == 1) );
  REMUS_ASSERT( (pool.supportedIOTypes().size() == 1) )

  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );

  //add the worker again with a different type and see if
  //we still only have a single worker that isn't ready for work
  pool.addWorker(worker1_id, worker_type3D);
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type3D) == true) );
  REMUS_ASSERT( (pool.allWorkers().count(worker1_id) == 1) );
  REMUS_ASSERT( (pool.supportedIOTypes().size() == 2) )

  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == false) );

  //verify that we have the worker as both 2d and 3d
  pool.readyForWork(worker1_id, worker_type2D);
  pool.readyForWork(worker1_id, worker_type3D);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == true) );
  REMUS_ASSERT( (pool.supportedIOTypes().size() == 2) )

}

void verify_has_worker_type()
{
  //verify that we only have workers for the given types
  //that we have added, and no false positives
  remus::server::detail::WorkerPool pool;
  zmq::SocketIdentity worker1_id = make_socketId();
  pool.addWorker(worker1_id, worker_type2D);

  //now verify that a worker added to the pool, but not marked as ready
  //for work, means that we are aware of a given type
  remus::common::MeshIOTypeSet typeSet = pool.supportedIOTypes();
  REMUS_ASSERT( (typeSet.size() == 1) )
  REMUS_ASSERT( (typeSet.count(worker_type2D.meshTypes()) == 1) );

  pool.readyForWork(worker1_id, worker_type2D);

  remus::common::MeshIOTypeSet allTypes = remus::common::generateAllIOTypes();
  typedef remus::common::MeshIOTypeSet::const_iterator cit;
  for( cit i = allTypes.begin(); i != allTypes.end(); ++i)
    {
    remus::common::MeshIOType io_type( *i );

    remus::proto::JobRequirements reqs( worker_type2D.formatType(),
                                        io_type,
                                        worker_type2D.workerName(),
                                        worker_type2D.requirements(),
                                        worker_type2D.requirementsSize()
                                       );
    bool valid = pool.haveWaitingWorker(reqs);
    //only when io_type equals
    REMUS_ASSERT( (valid == (reqs == worker_type2D) ) )
    }

  //now verify we can get the set of MeshIOTypes of worker ready for work
  REMUS_ASSERT( (pool.supportedIOTypes().size() == 1) )
  REMUS_ASSERT( (pool.supportedIOTypes().count(worker_type2D.meshTypes()) == 1) );

}

void verify_purge_workers()
{
  //verify that we properly purge workers given a time
  remus::server::detail::WorkerPool pool;
  zmq::SocketIdentity worker1_id = make_socketId();

  typedef remus::server::detail::SocketMonitor MonitorType;
  MonitorType monitor = make_Monitor( );

  pool.addWorker(worker1_id, worker_type2D);
  pool.addWorker(worker1_id, worker_type3D);
  pool.readyForWork(worker1_id, worker_type2D);
  pool.readyForWork(worker1_id, worker_type3D);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == true) );

  //refresh the worker, so that the monitor knows of its existence
  monitor.refresh(worker1_id);

  //fail to purge by using the time stamp the worker was added with
  pool.purgeDeadWorkers(monitor);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type3D) == true) );

  //mark workers as inactive and not ready for jobs by waiting 3 seconds
  remus::common::SleepForMillisec(3000);

  pool.purgeDeadWorkers(monitor);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == false) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type3D) == true) );

  REMUS_ASSERT( (pool.allWorkers().size() == 1) );
  REMUS_ASSERT( (pool.allResponsiveWorkers().size() == 0) );

  //refresh the work will make it active on the next check to purge workers
  monitor.refresh(worker1_id);
  pool.purgeDeadWorkers(monitor);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id, worker_type3D) == true) );

  REMUS_ASSERT( (pool.allWorkers().size() == 1) );
  REMUS_ASSERT( (pool.allResponsiveWorkers().size() == 1) );
  REMUS_ASSERT( (pool.allWorkersWantingWork().size() == 1) );
}

void verify_taking_works()
{
  remus::server::detail::WorkerPool pool;
  zmq::SocketIdentity worker1_id = make_socketId();

  //try to take a worker before it has been marked as ready for work
  pool.addWorker(worker1_id, worker_type2D);
  zmq::SocketIdentity bad_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( (bad_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( !(bad_id == worker1_id) );

  //verify that we can take workers for a given job type
  pool.readyForWork(worker1_id, worker_type2D);
  zmq::SocketIdentity good_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( !(good_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( (good_id == worker1_id) );
  REMUS_ASSERT( (pool.allWorkersWantingWork().size() == 0) );
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  //verify that we can take a worker that is registered for two different
  //types for one of those types and it will still be there for the other
  //type
  {
  pool.addWorker(worker1_id, worker_type2D);
  pool.addWorker(worker1_id, worker_type3D);
  pool.readyForWork(worker1_id, worker_type2D);
  pool.readyForWork(worker1_id, worker_type3D);

  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  zmq::SocketIdentity good_2d_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( !(good_2d_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( (good_2d_id == worker1_id) );
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  zmq::SocketIdentity bad_2d_id = pool.takeWorker(worker_type2D);
  zmq::SocketIdentity good_3d_id = pool.takeWorker(worker_type3D);

  REMUS_ASSERT( (bad_2d_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( !(bad_2d_id == worker1_id) );
  REMUS_ASSERT( !(good_3d_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( (good_3d_id == worker1_id) );
  REMUS_ASSERT( (pool.allWorkersWantingWork().size() == 0) );
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  //now verify that worker types we are aware of
  //this should be 2 since we have worker_type2D and worker_type3D
  //and while they have been assigned a job they still are in the pool
  //since they are still responsive to heartbeating
  REMUS_ASSERT( (pool.supportedIOTypes().size() == 2) )
  }

  //add a worker for 2 types, but only one of those types as
  //ready. and verify everything works properly
  {
  pool.addWorker(worker1_id, worker_type2D);
  pool.readyForWork(worker1_id, worker_type2D);
  pool.addWorker(worker1_id, worker_type3D);

  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  zmq::SocketIdentity good_2d_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( !(good_2d_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( (good_2d_id == worker1_id) );
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  zmq::SocketIdentity bad_2d_id = pool.takeWorker(worker_type2D);
  zmq::SocketIdentity bad_3d_id = pool.takeWorker(worker_type3D);

  REMUS_ASSERT( (bad_2d_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( !(bad_2d_id == worker1_id) );
  REMUS_ASSERT( (bad_3d_id == zmq::SocketIdentity()) );
  REMUS_ASSERT( !(bad_3d_id == worker1_id) );

  //still have the 3d worker item kicking around
  REMUS_ASSERT( (pool.allWorkers().size() == 1) );

  //now verify that worker types we are aware of
  REMUS_ASSERT( (pool.supportedIOTypes().size() == 2) )

  }
}

} //namespace

int UnitTestWorkerPool(int, char *[])
{
  verify_has_workers();

  verify_has_worker_type();

  verify_purge_workers();

  verify_taking_works();

  return 0;
}
