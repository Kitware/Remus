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

namespace remus{
namespace proto{

//------------------------------------------------------------------------------
Job::Job(const boost::uuids::uuid& my_id, const remus::common::MeshIOType& my_type):
    Id(my_id),
    Type(my_type)
  {
  }


//------------------------------------------------------------------------------
void Job::serialize(std::ostream& buffer) const
{
  buffer << this->type() << std::endl;
  buffer << this->id() << std::endl;
}

//------------------------------------------------------------------------------
Job::Job(std::istream& buffer):
  Id(), //need to default to null id
  Type() //needs to default to a bad mesh io type
{
  this->deserialize(buffer);
}

//------------------------------------------------------------------------------
Job::Job(const std::string& data):
  Id(), //need to default to null
  Type() //needs to default to a bad mesh io type
{
  std::stringstream buffer(data);
  this->deserialize(buffer);
}

//------------------------------------------------------------------------------
void Job::deserialize(std::istream& buffer)
{
  buffer >> this->Type;
  //While this is fairly complex, it is the only correct way I have found
  //to properly de-serialize a null boost id
  std::string id_as_string;
  buffer >> id_as_string;
  if(!id_as_string.empty())
    { //othewise the default null id is used. We have to use the c_str syntax here
      //as lexial_cast on certain versions of boost will trunacate the std::string
      //and than crash
    this->Id = boost::lexical_cast<boost::uuids::uuid>(id_as_string.c_str());
    }
}

}
}
