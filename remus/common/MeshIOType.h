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
    combined_type(0)
    {}

  MeshIOType(remus::MESH_INPUT_TYPE in, remus::MESH_OUTPUT_TYPE out)
    {
    Both.input = in;
    Both.output = out;
    }

  boost::int32_t type() const { return combined_type; }
  remus::MESH_INPUT_TYPE inputType() const
    { return static_cast<remus::MESH_INPUT_TYPE>(this->Both.input); }
  remus::MESH_OUTPUT_TYPE outputType() const
    { return static_cast<remus::MESH_OUTPUT_TYPE>(this->Both.output); }

  //since zero for input and out is invalid a combined value of
  //zero for the full int32 is also invalid
  bool valid() const { return combined_type != 0; }

  //needed to see if a client request type and a workers type are equal
  bool operator ==(const MeshIOType& b) const { return combined_type == b.combined_type; }

  //needed to properly store mesh types into stl containers
  bool operator <(const MeshIOType& b) const { return combined_type < b.combined_type; }

  //needed to encode the object on the wire
  friend std::ostream& operator<<(std::ostream &os, const MeshIOType &ctype)
    {
    os << ctype.type();
    return os;
    }

  //needed to decode the object from the wire
  friend std::istream& operator>>(std::istream &is, MeshIOType &ctype)
    {
    is >> ctype.combined_type;
    return is;
    }

protected:
  union {
    boost::int32_t combined_type;
    struct {
      boost::int16_t input;
      boost::int16_t output;
      } Both;
  };
};


}
}

#endif // remus_common_MeshIOType_h
