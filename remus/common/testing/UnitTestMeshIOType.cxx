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
#include <algorithm>

#include <remus/common/MeshTypes.h>
#include <remus/common/MeshIOType.h>

#include <remus/testing/Testing.h>


namespace
{
//lets derive a new type from mesh registry
struct TextMeshType : remus::meshtypes::MeshTypeBase
{
  static boost::shared_ptr<MeshTypeBase> create() {
          return boost::shared_ptr<TextMeshType>(new TextMeshType()); }
  std::string name() const { return "TextMeshType"; }
};

void RegisterTextMeshType()
{
  remus::common::MeshRegistrar a( (TextMeshType()) );
}

template<typename T, typename U, typename V>
void VerifyValid(T t, U u, V v)
{
  REMUS_ASSERT( (t.valid() == true) );
  REMUS_ASSERT( (t.inputType() == u.name()) );
  REMUS_ASSERT( (t.outputType() == v.name()) );
}

void verify_invalid_type()
{
  remus::meshtypes::Model m;
  remus::meshtypes::Mesh2D m2d;

  //verify that setting the inputType or outputType to MeshTypeBase makes
  //the meshIoType invalid
  remus::common::MeshIOType invalid( (remus::meshtypes::MeshTypeBase()),
                                     (remus::meshtypes::MeshTypeBase()) );

  REMUS_ASSERT ( (invalid.valid()==false) );

  remus::common::MeshIOType invalid2( (remus::meshtypes::MeshTypeBase()), m );
  REMUS_ASSERT ( (invalid2.valid()==false) );

  remus::common::MeshIOType invalid3( m2d, (remus::meshtypes::MeshTypeBase()) );
  REMUS_ASSERT ( (invalid3.valid()==false) );
}

void verify_custom_type()
{
  RegisterTextMeshType();

  remus::meshtypes::Model m;
  TextMeshType text;

  remus::common::MeshIOType test(m,text);
  VerifyValid(test, m, text);

  VerifyValid( (remus::common::MeshIOType(text,text)),
                text,
                (TextMeshType()) );

  //now verify that we can take a mesh type from the helper constructors
  { //verify string constructor
  remus::common::MeshIOType validMeshIOType(m.name(),text.name());
  VerifyValid( validMeshIOType, m, (TextMeshType()) );
  }

  { //verify shared_ptr constructor
  remus::common::MeshIOType validMeshIOType(
                remus::meshtypes::to_meshType(m.name()),
                remus::meshtypes::to_meshType(text.name()));
  VerifyValid( validMeshIOType, m, (TextMeshType()) );
  }
}

void verify_set()
{
  //verify that we can add meshtypes into set
  remus::common::MeshIOTypeSet type_set;

  type_set.insert( remus::common::MeshIOType(
                   remus::meshtypes::to_meshType("Mesh1D"),
                   remus::meshtypes::to_meshType("Mesh1D")
                   ) );
  //this one already exists, so the set should ignore it
  type_set.insert( remus::common::MeshIOType("Mesh1D","Mesh1D") );

  type_set.insert( remus::common::MeshIOType("Model","Mesh2D") );
  {
  remus::meshtypes::Model model;
  remus::meshtypes::Mesh2D mesh2d;
  type_set.insert( remus::common::MeshIOType(model,mesh2d) );
  }

  type_set.insert( remus::common::MeshIOType("Model","FOOType") );

  REMUS_ASSERT( (type_set.size() == 3) )

  //verify we can search it
  remus::common::MeshIOType valid_type("Model", "FOOType");
  remus::common::MeshIOType in_valid_type("FOOType", "Model");

  REMUS_ASSERT( (type_set.count( valid_type ) == 1 ) );
  REMUS_ASSERT( (type_set.count( in_valid_type ) == 0 ) );
}

void verify_serialization()
{
  //verify that we can add serialize a set, and as a bye product we verify
  //that we can serialize MeshIOType
  remus::common::MeshIOTypeSet to_wire;

  to_wire.insert( remus::common::MeshIOType("Model","FOOType") );
  to_wire.insert( remus::common::MeshIOType("Mesh2D","Model") );
  to_wire.insert( remus::common::MeshIOType("AbstractModel","AMR") );

  std::stringstream buffer;
  buffer << to_wire;

  remus::common::MeshIOTypeSet from_wire;
  buffer >> from_wire;

  const bool same = std::equal(to_wire.begin(),
                               to_wire.end(),
                               from_wire.begin());
  REMUS_ASSERT( same );
}

}


int UnitTestMeshIOType(int, char *[])
{
  verify_invalid_type();
  verify_custom_type();
  verify_set();
  verify_serialization();
  return 0;
}
