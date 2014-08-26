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
class MeshIOType
{
public:
  //construct an invalid mesh type
  MeshIOType():
    InputName(),
    OutputName()
    {}

  MeshIOType(const std::string& in, const std::string& out):
    InputName(in),
    OutputName(out)
    {
    }

  MeshIOType(const boost::shared_ptr<remus::meshtypes::MeshTypeBase>& in,
             const boost::shared_ptr<remus::meshtypes::MeshTypeBase>& out):
    InputName(in->name()),
    OutputName(out->name())
    {
    }


  MeshIOType(const remus::meshtypes::MeshTypeBase& in,
             const remus::meshtypes::MeshTypeBase& out):
    InputName(in.name()),
    OutputName(out.name())
    {
    }

  const std::string& inputType() const { return this->InputName; }
  const std::string& outputType() const { return this->OutputName; }

  //If either the input or output is invalid we need say we are invalid.
  //If we just check the combined type we only see if both are invalid.
  bool valid() const { return !inputType().empty() && !outputType().empty(); }

  //needed to see if a client request type and a workers type are equal
  bool operator ==(const MeshIOType& b) const
    {
    return (this->inputType() == b.inputType() &&
            this->outputType() == b.outputType());
    }

  //needed to properly store mesh types into stl containers
  bool operator <(const MeshIOType& b) const
    {
    if( this->inputType() == b.inputType() )
      { return ( this->outputType() < b.outputType() ); }
    else
      { return ( this->inputType() < b.inputType() ); }
    }

  //needed to encode the object on the wire
  friend std::ostream& operator<<(std::ostream &os, const MeshIOType &ctype)
    {
    os << ctype.inputType() << std::endl;
    os << ctype.outputType() << std::endl;
    return os;
    }

  //needed to decode the object from the wire
  friend std::istream& operator>>(std::istream &is, MeshIOType &ctype)
    {
    is >> ctype.InputName;
    is >> ctype.OutputName;
    return is;
    }

protected:
  std::string InputName;
  std::string OutputName;
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
