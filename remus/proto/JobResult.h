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

#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

//for ContentFormat and ContentSource
#include <remus/common/ContentTypes.h>
#include <remus/common/FileHandle.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif

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

  //pass in some data to send back to the client. The
  //contents of the string will be copied into a local memory
  //The result should be considered invalid if the contents length is zero
  JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const std::string& contents);

  //pass in some data to send back to the client.  A pointer
  //to the contents is kept and no copy will of data will happen, so
  //the contents of the pointer can't be deleted while the JobResult instance
  //is valid.
  JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const char* contents,
            std::size_t size);

  //get the storage format that we currently have setup for the source
  remus::common::ContentFormat::Type formatType() const
    { return this->FormatType; }

  bool valid() const;

  const boost::uuids::uuid& id() const { return JobId; }

  const char* data() const;
  std::size_t dataSize() const;


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
  friend REMUSPROTO_EXPORT remus::proto::JobResult to_JobResult(const char* data, std::size_t size);
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobResult(std::istream& buffer);

  boost::uuids::uuid JobId;
  remus::common::ContentFormat::Type FormatType;

  struct InternalImpl;
  boost::shared_ptr<InternalImpl> Implementation;
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
REMUSPROTO_EXPORT
std::string to_string(const remus::proto::JobResult& result);

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

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif


#endif
