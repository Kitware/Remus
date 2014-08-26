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

  //verify the iostream operator works
  std::cout << t << std::endl;
  }
}


int UnitTestMeshIOType(int, char *[])
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


  //verify that setting the inputType or outputType to MeshTypeBase makes
  //the meshIoType invalid
  remus::common::MeshIOType invalid( (remus::meshtypes::MeshTypeBase()),
                                     (remus::meshtypes::MeshTypeBase()) );

  REMUS_ASSERT ( (invalid.valid()==false) );

  remus::common::MeshIOType invalid2( (remus::meshtypes::MeshTypeBase()), m );
  REMUS_ASSERT ( (invalid2.valid()==false) );

  remus::common::MeshIOType invalid3( text, (remus::meshtypes::MeshTypeBase()) );
  REMUS_ASSERT ( (invalid3.valid()==false) );


  return 0;
}
