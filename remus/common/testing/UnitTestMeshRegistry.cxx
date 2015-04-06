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
  std::string name() const { return "TextMeshType"; }
  };

  void RegisterTextMeshType()
    {
    remus::common::MeshRegistrar a( (TextMeshType()) );
    REMUS_ASSERT( (remus::common::MeshRegistrar::numberOfRegisteredTypes() > 1) );
    }

  template<typename T>
  void VerifySame(T t1, T t2)
  {
    REMUS_ASSERT( (t1->name() == t2->name()) );
  }

  void verify_create()
  {
    using namespace remus::meshtypes;
    VerifySame(Mesh1D::create(), to_meshType("Mesh1D"));
    VerifySame(Mesh2D::create(), to_meshType("Mesh2D"));
    VerifySame(Mesh3D::create(), to_meshType("Mesh3D"));
    VerifySame(Mesh3DSurface::create(), to_meshType("Mesh3DSurface"));
    VerifySame(SceneFile::create(), to_meshType("SceneFile"));
    VerifySame(Model::create(), to_meshType("Model"));
    VerifySame(DiscreteModel::create(), to_meshType("DiscreteModel"));
    VerifySame(DiscreteModel1D::create(), to_meshType("DiscreteModel1D"));
    VerifySame(DiscreteModel2D::create(), to_meshType("DiscreteModel2D"));
    VerifySame(DiscreteModel3D::create(), to_meshType("DiscreteModel3D"));
    VerifySame(Edges::create(), to_meshType("Edges"));
    VerifySame(PiecewiseLinearComplex::create(), to_meshType("PiecewiseLinearComplex"));
  }

  void no_conflicting_ids_or_names()
  {
  typedef boost::shared_ptr<remus::meshtypes::MeshTypeBase> MeshType;

  std::set<MeshType> all_types = remus::common::MeshRegistrar::allRegisteredTypes();
  REMUS_ASSERT( (all_types.size() > 0) );
  REMUS_ASSERT( (all_types.size() == 12) );


  for(std::set<MeshType>::const_iterator i = all_types.begin();
      i != all_types.end();
      ++i)
    {
    //verify that the to_meshType works properly with any id and name
    VerifySame(*i,remus::meshtypes::to_meshType((*i)->name()));
    }

  std::set<std::string> names;
  for(std::set<MeshType>::const_iterator i = all_types.begin();
      i != all_types.end();
      ++i)
    {
    //verify we don't have two types with the same name or id
    names.insert((*i)->name());
    }
  REMUS_ASSERT( (all_types.size() == names.size()) );
  }

  void can_add_custom_type()
  {
    RegisterTextMeshType();

    typedef boost::shared_ptr<remus::meshtypes::MeshTypeBase> MeshType;
    MeshType base = remus::common::MeshRegistrar::instantiate("TextMeshType");

    VerifySame(base, TextMeshType::create() );
    VerifySame(base, remus::meshtypes::to_meshType("TextMeshType"));
  }
}



int UnitTestMeshRegistry(int, char *[])
{
  verify_create();
  no_conflicting_ids_or_names();
  can_add_custom_type();

  return 0;
}
