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

#include <remus/common/MeshTypes.h>
#include <remus/common/remusGlobals.h>

namespace remus {
namespace common {

//------------------------------------------------------------------------------
//MeshIOType is a union of the mesh input and output types into a single integer
//that represents the requirements of a mesh job.
//
//The single integer is used to compare if a workers submitted job request
//matches that of what any worker has registered support for.
struct MeshIOType
{

  //construct an invalid mesh type
  MeshIOType():
    CombinedType(0)
    {}

  MeshIOType(boost::shared_ptr<remus::meshtypes::MeshTypeBase> in,
             boost::shared_ptr<remus::meshtypes::MeshTypeBase> out)
    {
    this->CombinedType = out->id();
    this->CombinedType <<= 16; //shift out to the upper 16 bits
    this->CombinedType += in->id(); //now set the lower 16 bits to the value in
    }

  MeshIOType(const remus::meshtypes::MeshTypeBase& in,
             const remus::meshtypes::MeshTypeBase& out)
    {
    //set combined to out
    this->CombinedType = out.id();
    this->CombinedType <<= 16; //shift out to the upper 16 bits
    this->CombinedType += in.id(); //now set the lower 16 bits to the value in
    }

  boost::uint32_t type() const { return CombinedType; }

  boost::uint16_t inputType() const
    {
    return this->CombinedType & 0xFFFF;
    }

  boost::uint16_t outputType() const
    {
    return (this->CombinedType & (0xFFFF << 16)) >> 16;
    }

  //since zero for input and out is invalid a combined value of
  //zero for the full int32 is also invalid
  bool valid() const { return inputType() != 0 && outputType() !=0; }

  //needed to see if a client request type and a workers type are equal
  bool operator ==(const MeshIOType& b) const { return CombinedType == b.CombinedType; }

  //needed to properly store mesh types into stl containers
  bool operator <(const MeshIOType& b) const { return CombinedType < b.CombinedType; }

  //needed to encode the object on the wire
  friend std::ostream& operator<<(std::ostream &os, const MeshIOType &ctype)
    {
    os << ctype.type();
    return os;
    }

  //needed to decode the object from the wire
  friend std::istream& operator>>(std::istream &is, MeshIOType &ctype)
    {
    is >> ctype.CombinedType;
    return is;
    }

protected:
  boost::uint32_t CombinedType;
};

inline
remus::common::MeshIOType make_MeshIOType(
             const remus::meshtypes::MeshTypeBase& in,
             const remus::meshtypes::MeshTypeBase& out)
{
  return remus::common::MeshIOType(in,out);
}


}
}

#endif // remus_common_MeshIOType_h
