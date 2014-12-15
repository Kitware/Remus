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
#include <remus/server/WorkerFactory.h>
#include <remus/testing/Testing.h>

//configured file that gives us the path to the worker to test with
#include "UnitTestWorkerFactoryPaths.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
#  include <unistd.h> // for usleep
static void remusNap(int msec) { usleep(msec * 1000); }
#else
#  include <windows.h> // for Sleep
static void remusNap(int msec) { Sleep(msec); }
#endif

namespace {

using namespace remus::common;
using namespace remus::meshtypes;

template<typename In, typename Out>
remus::proto::JobRequirements make_Reqs(In in, Out out, const std::string& wname = "TestWorker")
{
  return remus::proto::make_JobRequirements( MeshIOType(in,out),
                                             wname,
                                             std::string());
}

void test_factory_constructors()
{
  //verify that all the constructors exist, and behave in the
  //expected manner. Note the class doesn't support copy
  //or move semantics currently
  remus::server::WorkerFactory f_def;
  remus::server::WorkerFactory f_copy = f_def;

  REMUS_ASSERT( (f_copy.workerExtension() == f_def.workerExtension()) );

  //verify the extension on the copy
  remus::server::WorkerFactory f_ext(".txt");
  f_copy = f_ext;

  REMUS_ASSERT( (f_copy.workerExtension() == ".txt") );
  REMUS_ASSERT( (f_copy.workerExtension() != f_def.workerExtension()));
}

void test_factory_worker_counts()
{
  remus::server::WorkerFactory f_def;
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
  //give our worker factory a unique extension to look for
  remus::server::WorkerFactory f_def(".tst");

  //add invalid paths to search

  f_def.addWorkerSearchDirectory(
                  remus::server::testing::worker_factory::locationToSearch() );

  //we should only support raw_edges and mesh2d, otherwise the rest
  //should return false
  remus::proto::JobRequirements raw_edges = make_Reqs(Edges(),Mesh2D());
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

void test_factory_worker_args_env_tag()
{
  //give our worker factory a unique extension to look for
  remus::server::WorkerFactory f_def(".aet");

  //add invalid paths to search

  f_def.addWorkerSearchDirectory(
                  remus::server::testing::worker_factory::locationToSearch() );

  //we should only support good and evil, otherwise the rest
  //should return false
  remus::proto::JobRequirements reqIOTypes = make_Reqs("Evil", "Good", "EvilToGoodWorker");

  // This is a bit of a hack: our job requirement tag() string must
  // exactly match the worker factory's string -- down to the whitespace
  // inserted by cJSON_Print
  reqIOTypes.tag("{\"thing\":\"yes\"}");
  REMUS_ASSERT( (f_def.haveSupport(reqIOTypes)) );

  //verify that factory reports reqIOTypes as a supportedIOType
  remus::common::MeshIOTypeSet validTypes = f_def.supportedIOTypes();
  REMUS_ASSERT( (validTypes.size() == 1) );
  REMUS_ASSERT( (validTypes.count(reqIOTypes.meshTypes()) == 1) );

  remus::common::MeshIOTypeSet allTypes = remus::common::generateAllIOTypes();
  typedef remus::common::MeshIOTypeSet::const_iterator cit;
  for( cit i = allTypes.begin(); i != allTypes.end(); ++i)
    {
    remus::proto::JobRequirements io_type = make_Reqs( (*i).inputType(),
                                                       (*i).outputType() );

    bool should_be_valid = (io_type == reqIOTypes);
    bool haveSupport_valid = f_def.haveSupport(io_type);
    bool workerReqs_valid =
            (f_def.workerRequirements(io_type.meshTypes()).size() > 0);
    //only when io_type equals
    REMUS_ASSERT( (haveSupport_valid == should_be_valid ) )
    REMUS_ASSERT( (workerReqs_valid  == should_be_valid ) )
    }

  remus::proto::JobRequirementsSet workerReqs =
                    f_def.workerRequirements(MeshIOType("Evil", "Good"));
  REMUS_ASSERT( (workerReqs.size() == 1) );

  remus::proto::JobRequirements w = *(workerReqs.begin());
  REMUS_ASSERT( (w.formatType() == ContentFormat::User) );
  REMUS_ASSERT( (w.sourceType() == ContentSource::Memory) );
  REMUS_ASSERT( (w.hasRequirements() == false) );

  const remus::server::WorkerFactoryBase::FactoryDeletionBehavior live =
    remus::server::WorkerFactoryBase::KillOnFactoryDeletion;

  //lets try to launch a worker with limit at 1
  f_def.setMaxWorkerCount(1);
  REMUS_ASSERT( (f_def.createWorker(reqIOTypes,live) == true) );

  //assert only 1 is created
  REMUS_ASSERT( (f_def.currentWorkerCount() == 1) );

  // Now wait around until it completes to verify that it ran.
  // This doesn't guarantee it was successful.
  while (f_def.currentWorkerCount() == 1)
    {
    remusNap(5);
    f_def.updateWorkerCount();
    }
}

void test_factory_worker_file_based_requirements()
{
  //give our worker factory a unique extension to look for
  remus::server::WorkerFactory f_def(".fbr");

  //add invalid paths to search

  f_def.addWorkerSearchDirectory(
                  remus::server::testing::worker_factory::locationToSearch() );

  MeshIOType raw_edges((Edges()),(Mesh2D()));
  REMUS_ASSERT( (f_def.workerRequirements(raw_edges).size() > 0) )

  //verify that factory reports raw_edges as a supportedIOType
  remus::common::MeshIOTypeSet validTypes = f_def.supportedIOTypes();
  REMUS_ASSERT( (validTypes.size() == 1) );
  REMUS_ASSERT( (validTypes.count(raw_edges) == 1) );

  std::set< MeshIOType > allTypes = remus::common::generateAllIOTypes();
  typedef std::set< MeshIOType >::const_iterator cit;
  for( cit i = allTypes.begin(); i != allTypes.end(); ++i)
    {
    const MeshIOType& io_type = *i;

    bool should_be_valid = (io_type == raw_edges);
    bool workerReqs_valid =
            (f_def.workerRequirements(io_type).size() > 0);
    //only when io_type equals
    REMUS_ASSERT( (workerReqs_valid  == should_be_valid ) )
    }




  remus::proto::JobRequirementsSet workerReqs =
                      f_def.workerRequirements(raw_edges);
  REMUS_ASSERT( (workerReqs.size() == 1) );

  remus::proto::JobRequirements w = *(workerReqs.begin());
  REMUS_ASSERT( (w.formatType() == ContentFormat::User) );
  REMUS_ASSERT( (w.sourceType() == ContentSource::File) );
  REMUS_ASSERT( (w.hasRequirements() == true) );
  REMUS_ASSERT( (w.requirementsSize() > 1) );
}


void test_factory_worker_invalid_paths()
{
  //give our worker factory a unique extension to look for
  remus::server::WorkerFactory f_def(".tst_asd");

  //add invalid paths to search
  f_def.addWorkerSearchDirectory( "/asdaaf/" );
  f_def.addWorkerSearchDirectory( "/asdzxasd2a" );

  //we should only support raw_edges and mesh2d, otherwise the rest
  //should return false
  remus::proto::JobRequirements raw_edges = make_Reqs(Edges(),Mesh2D());
  REMUS_ASSERT( (f_def.haveSupport(raw_edges) == 0) );
}

void test_factory_worker_launching()
{
  //give our worker factory a unique extension to look for
  const remus::server::WorkerFactoryBase::FactoryDeletionBehavior kill =
                remus::server::WorkerFactoryBase::KillOnFactoryDeletion;

  remus::server::WorkerFactory f_def(".tst");
  f_def.addCommandLineArgument("SLEEP_AND_EXIT");

  f_def.addWorkerSearchDirectory(
                  remus::server::testing::worker_factory::locationToSearch() );

  //we should only support raw_edges and mesh2d, otherwise the rest
  //should return false
  remus::proto::JobRequirements raw_edges = make_Reqs(Edges(),Mesh2D());

  REMUS_ASSERT( (f_def.haveSupport(raw_edges)) );

  f_def.setMaxWorkerCount(0);
  REMUS_ASSERT( (f_def.maxWorkerCount() == 0) );
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );

  //lets try to launch a worker with limit at zero
  REMUS_ASSERT( (f_def.createWorker(raw_edges,kill) == false) );

  //assert non have been created
  REMUS_ASSERT( (f_def.currentWorkerCount() == 0) );

  f_def.setMaxWorkerCount(1);
  REMUS_ASSERT( (f_def.maxWorkerCount() == 1) );

  //lets try to launch a worker with limit at 1
  REMUS_ASSERT( (f_def.createWorker(raw_edges,kill) == true) );

  //assert only 1 is created
  REMUS_ASSERT( (f_def.currentWorkerCount() == 1) );

  //try to make another, expected to fail
  REMUS_ASSERT( (f_def.createWorker(raw_edges,kill) == false) );

  // Now wait around until it completes to verify that it ran.
  // This doesn't guarantee it was successful.
  while (f_def.currentWorkerCount() == 1)
    {
    remusNap(5);
    f_def.updateWorkerCount();
    }
}


}//namespace


int UnitTestWorkerFactory(int, char *[])
{
  test_factory_constructors();

  test_factory_worker_counts();

  test_factory_worker_finder();

  test_factory_worker_file_based_requirements();

  test_factory_worker_args_env_tag();

  test_factory_worker_invalid_paths();

  test_factory_worker_launching();


  //if we have reached this line we have a proper server
  return 0;
}
