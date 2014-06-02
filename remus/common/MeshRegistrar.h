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
#ifndef remus_common_MeshRegistrar_h
#define remus_common_MeshRegistrar_h

#include <string>
#include <set>

//suppress warnings inside boost headers for gcc and clang
//as clang supports pragma GCC diagnostic
#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wcast-align"
#endif
#include <boost/cstdint.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <remus/common/CommonExports.h>

namespace remus {
namespace meshtypes {

//Remus reserves the right to first 100 meshtypes
//plugin mesh types should start at id 101
struct REMUSCOMMON_EXPORT MeshTypeBase
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

  bool operator <(boost::shared_ptr<remus::meshtypes::MeshTypeBase> b) const
    { return this->id() < b->id(); }
};

}
}

namespace remus {
namespace common {

struct REMUSCOMMON_EXPORT MeshRegistrar
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

  static std::size_t numberOfRegisteredTypes() {
    return NameRegistry().size();
  }

  static std::set<ReturnType> allRegisteredTypes()
  {
    std::set<ReturnType> result;
    boost::unordered_map<std::string, create_function_ptr>::const_iterator it;
    for(it = NameRegistry().begin(); it != NameRegistry().end(); ++it)
        { result.insert( (it->second)() ); }
    return result;
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