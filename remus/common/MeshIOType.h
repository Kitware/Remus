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

#ifndef remus_common_MeshIOType_h
#define remus_common_MeshIOType_h

//we include MeshTypes so that people can just include
//MeshIOType and get all the mesh types
#include <remus/common/MeshTypes.h>
#include <remus/common/remusGlobals.h>

#include <remus/common/CommonExports.h>


namespace remus {
namespace common {

//------------------------------------------------------------------------------
//MeshIOType is the input and output types of a given job type.
//These are used to describe worker types at a high level. For example
//this allows a server to state that it can transform Model into 3D Meshes
//
//
class REMUSCOMMON_EXPORT MeshIOType
{
public:
  //construct an invalid mesh type
  MeshIOType();

  MeshIOType(const std::string& in, const std::string& out);

  MeshIOType(const boost::shared_ptr<remus::meshtypes::MeshTypeBase>& in,
             const boost::shared_ptr<remus::meshtypes::MeshTypeBase>& out);

  MeshIOType(const remus::meshtypes::MeshTypeBase& in,
             const remus::meshtypes::MeshTypeBase& out);

  const std::string& inputType() const { return this->InputName; }
  const std::string& outputType() const { return this->OutputName; }

  //If either the input or output is invalid we need say we are invalid.
  //If we just check the combined type we only see if both are invalid.
  bool valid() const { return !inputType().empty() && !outputType().empty(); }

  //needed to see if a client request type and a workers type are equal
  bool operator ==(const MeshIOType& b) const;

  //needed to properly store mesh types into stl containers
  bool operator <(const MeshIOType& b) const;

  friend std::ostream& operator<<(std::ostream &os,
                                  const MeshIOType &types)
    { types.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  MeshIOType &types)
    { types = MeshIOType(is); return is; }

private:
  void serialize(std::ostream& buffer) const;
  explicit MeshIOType(std::istream& buffer);

  std::string InputName;
  std::string OutputName;
};

//a simple container so we can send a collection of MeshIOType
//to and from the client easily.
struct REMUSCOMMON_EXPORT MeshIOTypeSet
{
  typedef std::set< MeshIOType > ContainerType;

  typedef ContainerType::iterator iterator;
  typedef ContainerType::const_iterator const_iterator;
  typedef ContainerType::reference reference;
  typedef ContainerType::const_reference const_reference;
  typedef ContainerType::value_type value_type;

  MeshIOTypeSet();
  MeshIOTypeSet(const ContainerType& container);


  friend std::ostream& operator<<(std::ostream &os,
                                  const MeshIOTypeSet &reqs)
    { reqs.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  MeshIOTypeSet &reqs)
    { reqs = MeshIOTypeSet(is); return is; }

  std::pair<iterator,bool> insert( const value_type& value )
    { return this->Container.insert(value); }

  template< class InputIt >
  void insert( InputIt first, InputIt last )
    { return this->Container.insert(first,last); }

  iterator begin() { return this->Container.begin(); }
  const_iterator begin() const { return this->Container.begin(); }

  iterator end( ) { return this->Container.end(); }
  const_iterator end( ) const { return this->Container.end(); }

  std::size_t size() const { return this->Container.size(); }

  std::size_t count( const value_type& item ) const
    { return this->Container.count(item); }

private:
  void serialize(std::ostream& buffer) const;
  explicit MeshIOTypeSet(std::istream& buffer);

  ContainerType Container;
};

inline remus::common::MeshIOType make_MeshIOType(
             const remus::meshtypes::MeshTypeBase& in,
             const remus::meshtypes::MeshTypeBase& out)
{
  return remus::common::MeshIOType(in,out);
}

//helper method to generate the cross product of all known mesh io types
inline std::set< remus::common::MeshIOType > generateAllIOTypes()
{
  using namespace remus::common;
  using namespace remus::meshtypes;
  typedef boost::shared_ptr<remus::meshtypes::MeshTypeBase> MeshType;
  std::set<MeshType> all_types = remus::common::MeshRegistrar::allRegisteredTypes();

  std::set< MeshIOType > allIOTypes;

  typedef std::set<MeshType>::const_iterator cit;
  for(cit i=all_types.begin(); i!=all_types.end(); ++i)
    {
    for(cit j=all_types.begin(); j!=all_types.end(); ++j)
      {
      allIOTypes.insert( MeshIOType(*i,*j) );
      }
    }

  return allIOTypes;
}


}
}

#endif // remus_common_MeshIOType_h
