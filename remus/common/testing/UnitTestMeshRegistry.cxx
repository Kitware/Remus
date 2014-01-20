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
#include <remus/testing/Testing.h>


namespace
{
  //lets derive a new type from mesh registry
  struct TextMeshType : remus::meshtypes::MeshTypeBase
  {
  static boost::shared_ptr<MeshTypeBase> create() {
          return boost::shared_ptr<TextMeshType>(new TextMeshType()); }
  boost::uint16_t id() const { return 999; }
  std::string name() const { return "TextMeshType"; }
  };

  void RegisterTextMeshType()
    {
    remus::common::MeshRegistrar a( (TextMeshType()) );
    }

  template<typename T>
  void VerifySame(T t1, T t2)
  {
    std::cout << t1->name() << " and " << t2->name() << std::endl;
    REMUS_ASSERT( (t1->id() == t2->id()) );
    REMUS_ASSERT( (t1->name() == t2->name()) );
  }
}


int UnitTestMeshRegistry(int, char *[])
{
  RegisterTextMeshType();

  typedef boost::shared_ptr<remus::meshtypes::MeshTypeBase> MeshType;
  MeshType base = remus::common::MeshRegistrar::instantiate(999);

  VerifySame(base, TextMeshType::create() );
  VerifySame(base, remus::meshtypes::to_meshType(999) );
  VerifySame(base, remus::meshtypes::to_meshType("TextMeshType"));
  VerifySame(TextMeshType::create(), remus::meshtypes::to_meshType(base->id()));

  //verify a type that should have been added by the default static
  //initialization code
  base = remus::common::MeshRegistrar::instantiate(4);
  VerifySame(base, remus::meshtypes::Mesh3DSurface::create() );
  VerifySame(base, remus::meshtypes::to_meshType(4) );
  VerifySame(base, remus::meshtypes::to_meshType("Mesh3DSurface"));
  VerifySame(remus::meshtypes::Mesh3DSurface::create(),
             remus::meshtypes::to_meshType(base->id()));

  return 0;
}
