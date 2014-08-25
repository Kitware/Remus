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

namespace {

using namespace remus::common;
using namespace remus::meshtypes;

template<typename In, typename Out>
remus::proto::JobRequirements make_Reqs(In in, Out out)
{
  return remus::proto::make_JobRequirements( MeshIOType(in,out),
                                             std::string("TestWorker"),
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


  //what really we need are iterators to the mesh registrar
  const std::size_t num_mesh_types =
                    remus::common::MeshRegistrar::numberOfRegisteredTypes();

  for(std::size_t input_type = 1; input_type < num_mesh_types+1; ++input_type)
    {
    for(std::size_t output_type = 1; output_type < num_mesh_types+1; ++output_type)
      {
      remus::proto::JobRequirements io_type = make_Reqs(
                                    remus::meshtypes::to_meshType(input_type),
                                    remus::meshtypes::to_meshType(output_type));

      bool should_be_valid = (io_type == raw_edges);
      bool haveSupport_valid = f_def.haveSupport(io_type);
      bool workerReqs_valid =
              (f_def.workerRequirements(io_type.meshTypes()).size() > 0);
      //only when io_type equals
      REMUS_ASSERT( (haveSupport_valid == should_be_valid ) )
      REMUS_ASSERT( (workerReqs_valid  == should_be_valid ) )
      }
    }

  remus::proto::JobRequirementsSet workerReqs =
                    f_def.workerRequirements(MeshIOType((Edges()),(Mesh2D())));
  REMUS_ASSERT( (workerReqs.size() == 1) );

  remus::proto::JobRequirements w = *(workerReqs.begin());
  REMUS_ASSERT( (w.formatType() == ContentFormat::User) );
  REMUS_ASSERT( (w.sourceType() == ContentSource::Memory) );
  REMUS_ASSERT( (w.hasRequirements() == false) );

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

  //what really we need are iterators to the mesh registrar
  const std::size_t num_mesh_types =
                    remus::common::MeshRegistrar::numberOfRegisteredTypes();

  for(std::size_t input_type = 1; input_type < num_mesh_types+1; ++input_type)
    {
    for(std::size_t output_type = 1; output_type < num_mesh_types+1; ++output_type)
      {
      MeshIOType io_type = MeshIOType(remus::meshtypes::to_meshType(input_type),
                                      remus::meshtypes::to_meshType(output_type)
                                      );

      bool should_be_valid = (io_type == raw_edges);
      bool workerReqs_valid =
              (f_def.workerRequirements(io_type).size() > 0);
      //only when io_type equals
      REMUS_ASSERT( (workerReqs_valid  == should_be_valid ) )
      }
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
  const remus::server::WorkerFactory::FactoryDeletionBehavior kill =
                remus::server::WorkerFactory::KillOnFactoryDeletion;

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
}


}//namespace


int UnitTestWorkerFactory(int, char *[])
{
  test_factory_constructors();

  test_factory_worker_counts();

  test_factory_worker_finder();

  test_factory_worker_file_based_requirements();

  test_factory_worker_invalid_paths();

  test_factory_worker_launching();


  //if we have reached this line we have a proper server
  return 0;
}
