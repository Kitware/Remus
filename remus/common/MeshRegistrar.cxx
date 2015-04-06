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
#include <remus/common/MeshRegistrar.h>

//include the default mesh types
#include <remus/common/MeshTypes.h>

namespace
{
  //a work around so that we always have the default types added to the
  //mesh registry, no matter what. This does have a small performance
  //overhead
  void add_default_types()
  {
    static bool defaults_added  = false;
    if(!defaults_added)
      {
      //this needs to be set before we add any of the default types or
      //we will start an endless loop of calling this function.
      defaults_added = true;

      remus::common::MeshRegistrar a( (remus::meshtypes::Mesh1D()) );
      remus::common::MeshRegistrar b( (remus::meshtypes::Mesh2D()) );
      remus::common::MeshRegistrar c( (remus::meshtypes::Mesh3D()) );
      remus::common::MeshRegistrar d( (remus::meshtypes::Mesh3DSurface()) );
      remus::common::MeshRegistrar e( (remus::meshtypes::SceneFile()) );
      remus::common::MeshRegistrar f( (remus::meshtypes::Model()) );
      remus::common::MeshRegistrar g( (remus::meshtypes::DiscreteModel()) );
      remus::common::MeshRegistrar h( (remus::meshtypes::DiscreteModel1D()) );
      remus::common::MeshRegistrar i( (remus::meshtypes::DiscreteModel2D()) );
      remus::common::MeshRegistrar j( (remus::meshtypes::DiscreteModel3D()) );
      remus::common::MeshRegistrar k( (remus::meshtypes::Edges()) );
      remus::common::MeshRegistrar l( (remus::meshtypes::PiecewiseLinearComplex()) );

      }
  }
}

namespace remus {
namespace common {

MeshRegistrar::NameRegisteredMeshMapType & MeshRegistrar::NameRegistry()
{
  add_default_types();
  static MeshRegistrar::NameRegisteredMeshMapType NameImplementation;
  return NameImplementation;
}

}
}
