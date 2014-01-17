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
#ifndef __remus_common_MeshRegistrar_h
#define __remus_common_MeshRegistrar_h

#include <string>

#include <boost/cstdint.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

namespace remus {
namespace meshtypes {

struct MeshTypeBase
{
  //virtual destructor
  virtual ~MeshTypeBase() { }

  //virtual method to get back to the id of a mesh type.
  //The id is used to encode a mesh type for wire transfer
  virtual boost::uint16_t id() const { return 0; }

  //virtual method to get back of the name of the mesh type
  //The name is used to encode the mesh type when creating mesh
  //worker helper files
  virtual std::string name() const { return "Unknown"; }
};

}
}

namespace remus {
namespace common {

struct MeshRegistrar
{
private:
  //typedef of the create function ptr
  typedef boost::shared_ptr<remus::meshtypes::MeshTypeBase> ReturnType;
  typedef ReturnType (*create_function_ptr)();
public:

  template <typename D>
  explicit MeshRegistrar(D d)
    {
    MeshRegistrar::registrate(d.name(), d.id(), &D::create);
    }

  static ReturnType instantiate(std::string const & name)
    {
    boost::unordered_map<std::string,
        create_function_ptr>::const_iterator it = NameRegistry().find(name);
    return (it == NameRegistry().end()) ?
            ReturnType(new remus::meshtypes::MeshTypeBase()) : (it->second)();
    }

  static ReturnType instantiate(boost::uint16_t id)
    {
    boost::unordered_map<boost::uint16_t,
        create_function_ptr>::const_iterator it = IdRegistry().find(id);
    return (it == IdRegistry().end()) ?
            ReturnType(new remus::meshtypes::MeshTypeBase()) : (it->second)();
    }


private:
  static void registrate(std::string const & name, boost::uint16_t id,
                         create_function_ptr fp)
    {
    NameRegistry()[name] = fp;
    IdRegistry()[id] = fp;
    }


  typedef boost::unordered_map<std::string, create_function_ptr>
                                            NameRegisteredMeshMapType;
  static NameRegisteredMeshMapType& NameRegistry();

  typedef boost::unordered_map<boost::uint16_t, create_function_ptr>
                                            IdRegisteredMeshMapType;
  static IdRegisteredMeshMapType& IdRegistry();

  MeshRegistrar( const MeshRegistrar& other ); // non construction-copyable
  MeshRegistrar& operator=( const MeshRegistrar& ); // non copyable
};

}
}

#endif