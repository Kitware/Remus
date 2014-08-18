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

#include <string>
#include <remus/common/MeshRegistrar.h>

//Remus reserves the right to first 100 meshtypes
//plugin mesh types should start at id 101
namespace remus {
namespace meshtypes {

struct Mesh1D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh1D()); }
    boost::uint16_t id() const { return 1; }
    std::string name() const { return "Mesh1D"; }
};

struct Mesh2D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh2D()); }
    boost::uint16_t id() const { return 2; }
    std::string name() const { return "Mesh2D"; }
};

struct Mesh3D : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh3D()); }
    boost::uint16_t id() const { return 3; }
    std::string name() const { return "Mesh3D"; }
};

struct Mesh3DSurface : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Mesh3DSurface()); }
    boost::uint16_t id() const { return 4; }
    std::string name() const { return "Mesh3DSurface"; }
};

struct SceneFile : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new SceneFile()); }
    boost::uint16_t id() const { return 5; }
    std::string name() const { return "SceneFile"; }
};

struct Model : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Model()); }
    boost::uint16_t id() const { return 6; }
    std::string name() const { return "Model"; }
};

struct Edges : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new Edges()); }
    boost::uint16_t id() const { return 7; }
    std::string name() const { return "Edges"; }
};

struct PiecewiseLinearComplex : remus::meshtypes::MeshTypeBase
{
    static boost::shared_ptr<MeshTypeBase> create()
      { return boost::shared_ptr<MeshTypeBase>(new PiecewiseLinearComplex()); }
    boost::uint16_t id() const { return 8; }
    std::string name() const { return "PiecewiseLinearComplex"; }
};

inline
boost::shared_ptr<remus::meshtypes::MeshTypeBase>
to_meshType(const std::string& s)
{
  return remus::common::MeshRegistrar::instantiate(s);
}

inline
boost::shared_ptr<remus::meshtypes::MeshTypeBase>
to_meshType(boost::uint16_t t)
{
  return remus::common::MeshRegistrar::instantiate(t);
}

}
}


#endif //remus_common_MeshTypes_h
