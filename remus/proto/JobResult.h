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

#ifndef remus_proto_JobResult_h
#define remus_proto_JobResult_h

#include <string>
#include <sstream>

//for ContentFormat and ContentSource
#include <remus/common/ContentTypes.h>
#include <remus/common/FileHandle.h>

#include <boost/uuid/uuid.hpp>

//included for export symbols
#include <remus/proto/ProtoExports.h>

//Job result holds the result that the worker generated for a given job.
//The Data string will hold the actual job result, be it a file path or a custom
//serialized data structure.
namespace remus {
namespace proto {
class REMUSPROTO_EXPORT JobResult
{
public:
  //construct an invalid JobResult
  JobResult(const boost::uuids::uuid& jid);

  //pass in some data to send back to the client. The path to the file
  //will passed to the client. In the future we plan to extend remus
  //to support automatic file reading.
  //The result should be considered invalid if the file names length is zero
  JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const remus::common::FileHandle& fileHandle);

  //pass in some data back to the client. The
  //contents of the string will be copied into a local memory
  //The result should be considered invalid if the contents length is zero
  JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const std::string& contents);

  //get the storage format that we currently have setup for the source
  remus::common::ContentFormat::Type formatType() const
    { return this->FormatType; }

  bool valid() const { return Data.size() > 0; }

  const boost::uuids::uuid& id() const { return JobId; }
  const std::string& data() const { return Data; }

  //implement a less than operator and equal operator so you
  //can use the class in containers and algorithms
  bool operator<(const JobResult& other) const;
  bool operator==(const JobResult& other) const;

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobResult &submission)
    { submission.serialize(os); return os; }

  //needed to decode the object from the wire
  friend std::istream& operator>>(std::istream &is,
                                  JobResult &submission)
    { submission = JobResult(is); return is; }

private:
  friend remus::proto::JobResult to_JobResult(const char* data, std::size_t size);
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobResult(std::istream& buffer);

  boost::uuids::uuid JobId;
  remus::common::ContentFormat::Type FormatType;
  std::string Data; //data of the result of a job
};

//------------------------------------------------------------------------------
inline remus::proto::JobResult make_JobResult(const boost::uuids::uuid& id,
      const remus::common::FileHandle& handle,
      remus::common::ContentFormat::Type format = remus::common::ContentFormat::User)
{
  return remus::proto::JobResult(id,format,handle);
}

//------------------------------------------------------------------------------
inline remus::proto::JobResult make_JobResult(const boost::uuids::uuid& id,
      const std::string& content,
      remus::common::ContentFormat::Type format = remus::common::ContentFormat::User)
{
  return remus::proto::JobResult(id,format,content);
}

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobResult& result)
{
  std::ostringstream buffer;
  buffer << result;
  return buffer.str();
}

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT
remus::proto::JobResult to_JobResult(const char* data, std::size_t size);

//------------------------------------------------------------------------------
inline remus::proto::JobResult to_JobResult(const std::string& msg)
{
  return to_JobResult(msg.c_str(), msg.size());
}

}
}

#endif
