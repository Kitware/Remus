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

#include <algorithm>
#include <string>
#include <sstream>

#include <boost/uuid/uuid.hpp>

//suppress warnings inside boost headers for gcc, clang and MSVC
#ifndef _MSC_VER
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wshadow"
#else
# pragma warning(push)
//disable warning about using std::copy with pointers
# pragma warning(disable: 4996)
#endif
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#else
# pragma warning(pop)
#endif

#include <remus/common/MeshIOType.h>

//The remus::proto::Job class
// Holds the Id and Type of a submitted job.

namespace remus{
namespace proto{
class Job
{
public:

  //construct a valid job object with an Id and Type
  Job(const boost::uuids::uuid& my_id,
      const remus::common::MeshIOType& my_type):
  Id(my_id),
  Type(my_type)
  {
  }

  //get if the current job is a valid job
  bool valid() const { return Type.valid(); }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const { return Type; }

private:
  boost::uuids::uuid Id;
  remus::common::MeshIOType Type;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::Job& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::ostringstream buffer;
  buffer << job.type() << std::endl;
  buffer << job.id() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::proto::Job to_Job(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::istringstream buffer(msg);

  remus::common::MeshIOType type;
  buffer >> type;

  //While this is fairly complex, it is the only correct way I have found
  //to properly de-serialize a null boost id
  std::string id_as_string;
  buffer >> id_as_string;
  if(id_as_string.empty())
    {
    return remus::proto::Job(boost::uuids::uuid(),type);
    }
  boost::uuids::uuid id = boost::lexical_cast<boost::uuids::uuid>(id_as_string);
  return remus::proto::Job(id,type);
}


//------------------------------------------------------------------------------
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
