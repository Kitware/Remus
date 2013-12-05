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


#include <remus/common/zmqHelper.h>
#include <remus/server/detail/WorkerPool.h>
#include <remus/testing/Testing.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/random_generator.hpp>

namespace {
const remus::common::MeshIOType worker_type2D(remus::RAW_EDGES,
                                            remus::MESH2D);

const remus::common::MeshIOType worker_type3D(remus::RAW_EDGES,
                                             remus::MESH3D);

boost::uuids::random_generator generator;

//makes a random socket identity
zmq::socketIdentity make_socketId()
{
  boost::uuids::uuid new_uid = generator();
  const std::string str_id = boost::lexical_cast<std::string>(new_uid);
  return zmq::socketIdentity(str_id.c_str(),str_id.size());
}


void verify_has_workers()
{
  //verify that we start with zero workers
  remus::server::detail::WorkerPool pool;
  REMUS_ASSERT( (pool.livingWorkers().size() == 0) );

  //verify that if we add a worker we only have 1 worker,
  //and we have no workers ready for work
  zmq::socketIdentity worker1_id = make_socketId();
  pool.addWorker(worker1_id, worker_type2D);
  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == true) );
  REMUS_ASSERT( (pool.livingWorkers().count(worker1_id) == 1) );

  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );

  //add the worker again with a different type and see if
  //we still only have a single worker that isn't ready for work
  pool.addWorker(worker1_id, worker_type3D);
  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == true) );
  REMUS_ASSERT( (pool.livingWorkers().count(worker1_id) == 1) );

  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == false) );

  //verify that we have the worker as both 2d and 3d
  pool.readyForWork(worker1_id);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == true) );

}

void verify_has_worker_type()
{
  //verify that we only have workers for the given types
  //that we have added, and no false positives
  remus::server::detail::WorkerPool pool;
  zmq::socketIdentity worker1_id = make_socketId();
  pool.addWorker(worker1_id, worker_type2D);
  pool.readyForWork(worker1_id);

  for(remus::MESH_INPUT_TYPE input_type = remus::INVALID_MESH_IN;
    input_type != remus::NUM_MESH_INPUT_TYPES;
    input_type = remus::MESH_INPUT_TYPE((int)input_type+1))
    {
    for(remus::MESH_OUTPUT_TYPE output_type = remus::INVALID_MESH_OUT;
        output_type != remus::NUM_MESH_OUTPUT_TYPES;
        output_type = remus::MESH_OUTPUT_TYPE((int)output_type+1))
      {
      remus::common::MeshIOType io_type(input_type,output_type);
      bool valid = pool.haveWaitingWorker(io_type);
      //only when io_type equals
      REMUS_ASSERT( (valid == (io_type == worker_type2D) ) )
      }
    }

}

void verify_purge_workers()
{
  //verify that we properly purge workers given a time
  using namespace boost::posix_time;
  using namespace boost::gregorian;

  remus::server::detail::WorkerPool pool;
  zmq::socketIdentity worker1_id = make_socketId();


  const ptime long_ago = second_clock::local_time() -  days(10);
  const ptime future = second_clock::local_time() +  days(10);
  const ptime current = second_clock::local_time();

  pool.addWorker(worker1_id, worker_type2D);
  pool.addWorker(worker1_id, worker_type3D);
  pool.readyForWork(worker1_id);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == true) );

  //fail to purge by using the time stamp the worker was added with
  pool.purgeDeadWorkers(current);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == true) );

  //fail to purge by using a really old time stamp
  pool.purgeDeadWorkers(long_ago);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == true) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == true) );

  //purge the worker by using a time stamp in the future
  pool.purgeDeadWorkers(future);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == false) );

  //calling ready for work on an Id that isn't part of the bool
  //shouldn't add it to the pool
  pool.readyForWork(worker1_id);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type3D) == false) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == false) );

  pool.refreshWorker(worker1_id);
  REMUS_ASSERT( (pool.haveWaitingWorker(worker_type2D) == false) );
  REMUS_ASSERT( (pool.haveWorker(worker1_id) == false) );

  REMUS_ASSERT( (pool.livingWorkers().size() == 0) );
}

void verify_taking_works()
{
  remus::server::detail::WorkerPool pool;
  zmq::socketIdentity worker1_id = make_socketId();

  //try to take a worker before it has been marked as ready for work
  pool.addWorker(worker1_id, worker_type2D);
  zmq::socketIdentity bad_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( (bad_id == zmq::socketIdentity()) );
  REMUS_ASSERT( !(bad_id == worker1_id) );

  //verify that we can take workers for a given job type
  pool.readyForWork(worker1_id);
  zmq::socketIdentity good_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( !(good_id == zmq::socketIdentity()) );
  REMUS_ASSERT( (good_id == worker1_id) );
  REMUS_ASSERT( (pool.livingWorkers().size() == 0) );

  //verify that we can take a worker that is registered for two different
  //types for one of those types and it will still be there for the other
  //type
  {
  pool.addWorker(worker1_id, worker_type2D);
  pool.addWorker(worker1_id, worker_type3D);
  pool.readyForWork(worker1_id);
  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );

  zmq::socketIdentity good_2d_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( !(good_2d_id == zmq::socketIdentity()) );
  REMUS_ASSERT( (good_2d_id == worker1_id) );
  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );

  zmq::socketIdentity bad_2d_id = pool.takeWorker(worker_type2D);
  zmq::socketIdentity good_3d_id = pool.takeWorker(worker_type3D);

  REMUS_ASSERT( (bad_2d_id == zmq::socketIdentity()) );
  REMUS_ASSERT( !(bad_2d_id == worker1_id) );
  REMUS_ASSERT( !(good_3d_id == zmq::socketIdentity()) );
  REMUS_ASSERT( (good_3d_id == worker1_id) );
  REMUS_ASSERT( (pool.livingWorkers().size() == 0) );
  }

  //add a worker for 2 types, but only one of those types as
  //ready. and verify everything works properly
  {
  pool.addWorker(worker1_id, worker_type2D);
  pool.readyForWork(worker1_id);
  pool.addWorker(worker1_id, worker_type3D);

  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );

  zmq::socketIdentity good_2d_id = pool.takeWorker(worker_type2D);
  REMUS_ASSERT( !(good_2d_id == zmq::socketIdentity()) );
  REMUS_ASSERT( (good_2d_id == worker1_id) );
  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );

  zmq::socketIdentity bad_2d_id = pool.takeWorker(worker_type2D);
  zmq::socketIdentity bad_3d_id = pool.takeWorker(worker_type3D);

  REMUS_ASSERT( (bad_2d_id == zmq::socketIdentity()) );
  REMUS_ASSERT( !(bad_2d_id == worker1_id) );
  REMUS_ASSERT( (bad_2d_id == zmq::socketIdentity()) );
  REMUS_ASSERT( !(bad_2d_id == worker1_id) );

  //still have the 3d worker item kicking around
  REMUS_ASSERT( (pool.livingWorkers().size() == 1) );
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
