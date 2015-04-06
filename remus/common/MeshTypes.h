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

#ifndef remus_common_MeshTypes_h
#define remus_common_MeshTypes_h

#include <remus/common/MeshRegistrar.h>

//Remus reserves the right to first 100 meshtypes
//plugin mesh types should start at id 101
namespace remus {
namespace meshtypes {

struct Mesh1D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh1D()); }
    std::string name() const { return "Mesh1D"; }
};

struct Mesh2D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh2D()); }
    std::string name() const { return "Mesh2D"; }
};

struct Mesh3D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh3D()); }
    std::string name() const { return "Mesh3D"; }
};

struct Mesh3DSurface : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh3DSurface()); }
    std::string name() const { return "Mesh3DSurface"; }
};

struct SceneFile : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new SceneFile()); }
    std::string name() const { return "SceneFile"; }
};

struct Model : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Model()); }
    std::string name() const { return "Model"; }
};

struct DiscreteModel : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new DiscreteModel()); }
    std::string name() const { return "DiscreteModel"; }
};

struct DiscreteModel1D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new DiscreteModel1D()); }
    std::string name() const { return "DiscreteModel1D"; }
};

struct DiscreteModel2D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new DiscreteModel2D()); }
    std::string name() const { return "DiscreteModel2D"; }
};

struct DiscreteModel3D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new DiscreteModel3D()); }
    std::string name() const { return "DiscreteModel3D"; }
};

struct Edges : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Edges()); }
    std::string name() const { return "Edges"; }
};

struct PiecewiseLinearComplex : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new PiecewiseLinearComplex()); }
    std::string name() const { return "PiecewiseLinearComplex"; }
};

inline boost::shared_ptr<remus::meshtypes::MeshTypeBase>
to_meshType(const std::string& s)
{
  return remus::common::MeshRegistrar::instantiate(s);
}

}
}


#endif //remus_common_MeshTypes_h
