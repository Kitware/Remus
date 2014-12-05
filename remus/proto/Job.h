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

#ifndef remus_proto_Job_h
#define remus_proto_Job_h

#include <boost/uuid/uuid.hpp>
#include <remus/common/MeshIOType.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

namespace remus{
namespace proto{

//The remus::proto::Job class
// Holds the Id and Type of a submitted job.
class REMUSPROTO_EXPORT Job
{
public:

  //construct a valid job object with an Id and Type
  Job(const boost::uuids::uuid& my_id,
      const remus::common::MeshIOType& my_type);

  //get if the current job is a valid job
  bool valid() const { return Type.valid(); }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const { return Type; }

  bool operator ==(const Job& b) const
    { return this->Id == b.Id && this->Type == b.Type; }

  bool operator !=(const Job& b) const
    { return !(this->operator ==(b)); }

   friend std::ostream& operator<<(std::ostream &os, const Job &prog)
    { prog.serialize(os); return os; }
  friend std::istream& operator>>(std::istream &is, Job &prog)
    { prog = Job(is); return is; }

private:
  friend remus::proto::Job to_Job(const std::string& msg);
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit Job(std::istream& buffer);
  explicit Job(const std::string& data);
  void deserialize(std::istream& buffer);


  boost::uuids::uuid Id;
  remus::common::MeshIOType Type;
  std::string CachedSerializedForm;
};

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT std::string to_string(const remus::proto::Job& job);

//------------------------------------------------------------------------------
inline remus::proto::Job to_Job(const std::string& msg)
{
  return remus::proto::Job(msg);
}

//-----------------------------------------------------------------------------
inline remus::proto::Job to_Job(const char* data, std::size_t size)
{
  const std::string temp(data,size);
  return to_Job( temp );
}

//------------------------------------------------------------------------------
inline remus::proto::Job make_invalidJob()
{
  //use empty strings to signify invalid mesh io type
  const remus::common::MeshIOType badIOType( (std::string()), (std::string()) );
  return remus::proto::Job(boost::uuids::uuid(),badIOType);
}

}
}

#endif
