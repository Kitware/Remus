/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/

#ifndef __meshserver_internal_messageHelper_h
#define __meshserver_internal_messageHelper_h


#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp> //needed to get to_string
#include <string>

#include <meshserver/internal/JobMessage.h>

//The purpose of this header is to limit the number of files that
//need to include boost, plus give better names to the conversion from and too
//boost::uuid

namespace meshserver
{

//------------------------------------------------------------------------------
inline std::string to_string(const boost::uuids::uuid& id)
{
  //call the boost to_string method in uuid_io
  return boost::lexical_cast<std::string>(id);
}

//------------------------------------------------------------------------------
inline boost::uuids::uuid to_uuid(const meshserver::internal::JobMessage& msg)
{
  //take the contents of the msg and convert it to an uuid
  //no type checking will be done to make sure this is valid for now
  const std::string sId(msg.data(),msg.dataSize());
  return boost::lexical_cast<boost::uuids::uuid>(sId);
}

}

#endif // __meshserver_internal_messageHelper_h
