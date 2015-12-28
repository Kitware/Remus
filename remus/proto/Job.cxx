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

#include <remus/proto/Job.h>

#include <algorithm>
#include <string>
#include <sstream>

//suppress warnings inside boost headers for gcc, clang and MSVC
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace remus{
namespace proto{

namespace
{
  std::string make_serialzied_form( const boost::uuids::uuid& my_id,
                                    const remus::common::MeshIOType& my_type)
  {
    std::ostringstream buffer;
    buffer << my_type << std::endl;
    buffer << my_id << std::endl;
    return buffer.str();
  }
}

//------------------------------------------------------------------------------
Job::Job(const boost::uuids::uuid& my_id, const remus::common::MeshIOType& my_type):
    Id(my_id),
    Type(my_type),
    CachedSerializedForm( make_serialzied_form(my_id, my_type) )
{
}

//------------------------------------------------------------------------------
void Job::serialize(std::ostream& buffer) const
{
  buffer << this->CachedSerializedForm << std::endl;
}

//------------------------------------------------------------------------------
Job::Job(std::istream& buffer):
  Id(), //need to default to null id
  Type(), //needs to default to a bad mesh io type
  CachedSerializedForm()
{
  this->deserialize(buffer);
}

//------------------------------------------------------------------------------
Job::Job(const std::string& data):
  Id(), //need to default to null
  Type(), //needs to default to a bad mesh io type
  CachedSerializedForm(data)
{
  std::stringstream buffer(data);
  this->deserialize(buffer);
}

//------------------------------------------------------------------------------
void Job::deserialize(std::istream& buffer)
{
  buffer >> this->Type;
  // Deserialize a boost UUID using the string generator.
  // Invalid strings result in a NULL UUID.
  std::string id_as_string;
  buffer >> id_as_string;
  boost::uuids::string_generator sgen;
  this->Id = sgen(id_as_string);

  std::ostringstream cache_buffer;
  cache_buffer << *this;
  this->CachedSerializedForm = cache_buffer.str();
}

//------------------------------------------------------------------------------
std::string to_string(const remus::proto::Job& job)
{
  //convert a job to a string.
  std::ostringstream buffer;
  buffer << job;
  return buffer.str();
}

}
}
