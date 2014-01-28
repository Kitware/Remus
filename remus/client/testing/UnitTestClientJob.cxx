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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <remus/client/Job.h>

#include <remus/testing/Testing.h>

namespace
{
using namespace remus::client;
using namespace remus::common;
boost::uuids::random_generator generator;

void validate_serialization(Job s)
{
  std::string temp = to_string(s);
  Job from_string = to_Job(temp);

  REMUS_ASSERT( (from_string.id() == s.id()) );
  REMUS_ASSERT( (from_string.type() == s.type()) );
  REMUS_ASSERT( (from_string.valid() == s.valid()) );
}

}

int UnitTestClientJob(int, char *[])
{
  remus::meshtypes::Model model;
  remus::meshtypes::Mesh3D m3d;

  Job modelToMesh(generator(), MeshIOType(model,m3d) );
  Job modelToMesh2(modelToMesh.id(), MeshIOType(model,m3d) );

  Job meshToModel(generator(), MeshIOType(m3d,model) );
  Job meshToModel2(meshToModel.id(), MeshIOType(m3d,model) );


  Job modelToMesh3(meshToModel.id(), MeshIOType(model,m3d) );
  Job meshToModel3(modelToMesh2.id(), MeshIOType(m3d,model) );

  REMUS_ASSERT( (modelToMesh.id() == modelToMesh2.id()) );
  REMUS_ASSERT( (modelToMesh.id() != modelToMesh3.id()) );

  REMUS_ASSERT( (meshToModel.id() == meshToModel2.id()) );
  REMUS_ASSERT( (meshToModel.id() != meshToModel3.id()) );


  REMUS_ASSERT( (modelToMesh.type() == modelToMesh2.type()) );
  REMUS_ASSERT( (modelToMesh.type() == modelToMesh3.type()) );

  REMUS_ASSERT( (meshToModel.type() == meshToModel2.type()) );
  REMUS_ASSERT( (meshToModel.type() == meshToModel3.type()) );


  validate_serialization(modelToMesh);
  validate_serialization(modelToMesh2);
  validate_serialization(modelToMesh3);
  validate_serialization(meshToModel);
  validate_serialization(meshToModel2);
  validate_serialization(meshToModel3);


  return 0;
}