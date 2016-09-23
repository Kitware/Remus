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

#include <iostream>
#include <functional>
#include <remus/server/ThreadWorkerFactory.h>
#include <remus/testing/Testing.h>
#include <remus/common/SleepFor.h>

namespace
{
using namespace remus::common;
using namespace remus::meshtypes;

template<typename In, typename Out>
remus::proto::JobRequirements make_Reqs(In in, Out out, const std::string& wname = "TestWorker")
{
  return remus::proto::make_JobRequirements( MeshIOType(in,out),
                                             wname,
                                             std::string());
}

struct PrintJobRequirementAndEndpoint
{
  void operator()(const remus::proto::JobRequirements& j,
                  const std::string& endpoint) const
  {
    std::cout<<"Job requirements: "<<to_string(j)<<std::endl;
    std::cout<<"endpoint: "<<endpoint<<std::endl;
  }
};

struct Sleep
{
  void operator()(const remus::proto::JobRequirements& j,
                  const std::string& endpoint,
                  std::size_t sleep_millisec) const
  {
    std::cout<<"Job requirements: "<<to_string(j)<<std::endl;
    std::cout<<"endpoint: "<<endpoint<<std::endl;
    SleepForMillisec(static_cast<unsigned int>(sleep_millisec));
  }
};

void test_factory_constructors()
{
  //verify that all the constructors exist, and behave in the
  //expected manner. Note the class doesn't support copy
  //or move semantics currently
  remus::server::ThreadWorkerFactory f_def;
  REMUS_ASSERT( (f_def.maxWorkerCount() == 1) );
}

void test_factory_worker_counts()
{
  remus::server::ThreadWorkerFactory f_def;
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );
  REMUS_ASSERT( (f_def.currentWorkerCount() <  f_def.maxWorkerCount()) );

  //max worker count can't be negative, so lets test that.
  f_def.setMaxWorkerCount(-1); //roll it over to a large unsigned int
  REMUS_ASSERT( (f_def.maxWorkerCount() > 0) ); //verify it is a large value
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );

  f_def.setMaxWorkerCount(12);
  REMUS_ASSERT( (f_def.maxWorkerCount() == 12) );
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );
  //verify that checking for dead workers when we have no workers does nothing
  f_def.updateWorkerCount();
  REMUS_ASSERT( (f_def.maxWorkerCount() == 12) );
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );
}

void test_factory_worker_finder()
{
  // create a thread worker factory
  remus::server::ThreadWorkerFactory f_def;

  remus::proto::JobRequirements raw_edges = make_Reqs(Edges(),Mesh2D());

  f_def.registerWorkerType( raw_edges,
                            PrintJobRequirementAndEndpoint() );

  //we should only support raw_edges and mesh2d, otherwise the rest
  //should return false
  REMUS_ASSERT( (f_def.haveSupport(raw_edges)) );

  //verify that factory reports raw_edges as a supportedIOType
  remus::common::MeshIOTypeSet validTypes = f_def.supportedIOTypes();
  REMUS_ASSERT( (validTypes.size() == 1) );
  REMUS_ASSERT( (validTypes.count(raw_edges.meshTypes()) == 1) );

  remus::common::MeshIOTypeSet allTypes = remus::common::generateAllIOTypes();
  typedef remus::common::MeshIOTypeSet::const_iterator cit;
  for( cit i = allTypes.begin(); i != allTypes.end(); ++i)
    {
    remus::proto::JobRequirements io_type = make_Reqs( (*i).inputType(),
                                                       (*i).outputType() );

    bool should_be_valid = (io_type == raw_edges);
    bool haveSupport_valid = f_def.haveSupport(io_type);
    bool workerReqs_valid =
            (f_def.workerRequirements(io_type.meshTypes()).size() > 0);
    //only when io_type equals
    REMUS_ASSERT( (haveSupport_valid == should_be_valid ) )
    REMUS_ASSERT( (workerReqs_valid  == should_be_valid ) )
    }

  remus::proto::JobRequirementsSet workerReqs =
                    f_def.workerRequirements(MeshIOType((Edges()),(Mesh2D())));
  REMUS_ASSERT( (workerReqs.size() == 1) );

  remus::proto::JobRequirements w = *(workerReqs.begin());
  REMUS_ASSERT( (w.formatType() == ContentFormat::User) );
  REMUS_ASSERT( (w.sourceType() == ContentSource::Memory) );
  REMUS_ASSERT( (w.hasRequirements() == false) );

}

void test_factory_worker_launching()
{
  //give our worker factory a unique extension to look for
  const remus::server::WorkerFactoryBase::FactoryDeletionBehavior kill =
                remus::server::WorkerFactoryBase::KillOnFactoryDeletion;

  remus::server::ThreadWorkerFactory f_def;

  //we should only support raw_edges and mesh2d, otherwise the rest
  //should return false
  remus::proto::JobRequirements raw_edges = make_Reqs(Edges(),Mesh2D());

  f_def.registerWorkerType( raw_edges,
                            PrintJobRequirementAndEndpoint() );

  REMUS_ASSERT( (f_def.haveSupport(raw_edges)) );

  f_def.setMaxWorkerCount(0);
  REMUS_ASSERT( (f_def.maxWorkerCount() == 0) );
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );

  //lets try to launch a worker with limit at zero
  REMUS_ASSERT( (f_def.createWorker(raw_edges,kill) == false) );

  //assert none have been created
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );

  f_def.setMaxWorkerCount(1);
  REMUS_ASSERT( (f_def.maxWorkerCount() == 1) );

  //lets try to launch a worker with limit at 1
  REMUS_ASSERT( (f_def.createWorker(raw_edges,kill) == true) );
}

void test_shutdown_with_active_killOnFactoryDel_workers()
{
  //give our worker factory a unique extension to look for
  const remus::server::WorkerFactoryBase::FactoryDeletionBehavior kill =
                remus::server::WorkerFactoryBase::KillOnFactoryDeletion;

  remus::server::ThreadWorkerFactory f_def;
  f_def.setMaxWorkerCount(1);

  //we should only support raw_edges and mesh2d, otherwise the rest
  //should return false
  remus::proto::JobRequirements raw_edges = make_Reqs(Edges(),Mesh2D());

  f_def.registerWorkerType(raw_edges,
                           std::bind(Sleep(),
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     1000));

  REMUS_ASSERT( (f_def.haveSupport(raw_edges)) );

  //lets try to launch a worker with limit at 1
  REMUS_ASSERT( (f_def.createWorker(raw_edges,kill) == true) );

  //assert only 1 is created
  REMUS_ASSERT( (f_def.currentWorkerCount() == 1) );

  //now exit with worker still active
}

}//namespace


int UnitTestThreadWorkerFactory(int, char *[])
{
  test_factory_constructors();

  test_factory_worker_counts();

  test_factory_worker_finder();

  test_factory_worker_launching();

  test_shutdown_with_active_killOnFactoryDel_workers();

  //if we have reached this line we have a proper worker factory
  return 0;
}
