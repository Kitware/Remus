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

#ifndef remus_server_detail_uuidHelper_h
#define remus_server_detail_uuidHelper_h

#include <remus/common/CompilerInformation.h>
#include <remus/proto/Message.h>

//suppress warnings inside boost headers for gcc and clang
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace remus
{

//------------------------------------------------------------------------------
/**\brief Convert a UUID into a string.
  *
  */
inline std::string to_string(const boost::uuids::uuid& id)
{
  //call the boost to_string method in uuid_io
  return boost::lexical_cast<std::string>(id);
}

//------------------------------------------------------------------------------
/**\brief Convert the contents of a Remus Message to a UUID.
  *
  * The string generator returns a null UUID for an invalid string.
  */
inline boost::uuids::uuid to_uuid(const remus::proto::Message& msg)
{
  boost::uuids::string_generator gen;
  const std::string sId(msg.data(),msg.dataSize());
  return gen(sId);
}

//------------------------------------------------------------------------------
/**\brief Convert a string to a UUID.
  *
  * The string generator returns a null UUID for an invalid string.
  */
inline boost::uuids::uuid to_uuid(const std::string& str)
{
  boost::uuids::string_generator gen;
  return gen(str);
}
}

#endif // remus_server_detail_uuidHelper_h
