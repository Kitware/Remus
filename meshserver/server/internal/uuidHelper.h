/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_server_internal_uuidHelper_h
#define __meshserver_server_internal_uuidHelper_h


#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <meshserver/JobMessage.h>

namespace meshserver
{

//------------------------------------------------------------------------------
inline std::string to_string(const boost::uuids::uuid& id)
{
  //call the boost to_string method in uuid_io
  return boost::lexical_cast<std::string>(id);
}

//------------------------------------------------------------------------------
inline boost::uuids::uuid to_uuid(const meshserver::JobMessage& msg)
{
  //take the contents of the msg and convert it to an uuid
  //no type checking will be done to make sure this is valid for now
  const std::string sId(msg.data(),msg.dataSize());
  return boost::lexical_cast<boost::uuids::uuid>(sId);
}

//------------------------------------------------------------------------------
inline boost::uuids::uuid to_uuid(const std::string& str)
{
  return boost::lexical_cast<boost::uuids::uuid>(str);
}
}

#endif // __meshserver_server_internal_uuidHelper_h
