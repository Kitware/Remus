/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_common_messageHelper_h
#define __meshserver_common_messageHelper_h


#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp> //needed to get to_string

#include <string>
#include <sstream>

#include <meshserver/common/JobDetails.h>
#include <meshserver/common/JobStatus.h>
#include <meshserver/JobMessage.h>

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
inline boost::uuids::uuid to_uuid(const meshserver::JobMessage& msg)
{
  //take the contents of the msg and convert it to an uuid
  //no type checking will be done to make sure this is valid for now
  const std::string sId(msg.data(),msg.dataSize());
  return boost::lexical_cast<boost::uuids::uuid>(sId);
}

//------------------------------------------------------------------------------
inline std::string to_string(const meshserver::common::JobStatus& status)
{
  //convert a job status to a string, used as a hack to serialize
  std::stringstream buffer;
  buffer << status.JobId << std::endl;
  buffer << status.Status << std::endl;
  buffer << status.Progress << std::endl;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline std::string to_string(const meshserver::common::JobDetails& status)
{
  //convert a job detail to a string, used as a hack to serialize
  //encoding is simple, contents newline seperated
  std::stringstream buffer;
  buffer << status.Id << std::endl;
  buffer << status.Path << std::endl;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline meshserver::common::JobStatus to_JobStatus(const std::string& status)
{
  //convert a job status from a string, used as a hack to serialize
  std::stringstream buffer(status);

  std::string id;
  int t;
  int p;

  buffer >> id;
  buffer >> t;
  buffer >> p;

  const meshserver::STATUS_TYPE type = static_cast<meshserver::STATUS_TYPE>(t);
  return meshserver::common::JobStatus(id,type,p);
}

//------------------------------------------------------------------------------
inline meshserver::common::JobDetails to_JobDetails(const std::string& status)
{
  //convert a job detail from a string, used as a hack to serialize

  std::stringstream buffer(status);

  std::string id;
  std::string path;
  buffer >> id;
  buffer >> path;
  return meshserver::common::JobDetails(id,path);
}


}

#endif // __meshserver_common_messageHelper_h
