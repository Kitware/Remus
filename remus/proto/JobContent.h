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

#ifndef remus_proto_JobContent_h
#define remus_proto_JobContent_h

#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

//for ContentFormat and ContentSource
#include <remus/common/ContentTypes.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

namespace remus{
namespace proto{

class REMUSPROTO_EXPORT JobContent
{
public:
  //construct an invalid JobContent. This constructor is designed
  //to allows this class to be stored in containers.
  JobContent();

  //pass in some data to send to the worker. The
  //contents of the string will be copied into a local memory if the
  //data type is Memory. If the data type is file we will read the contents
  //of the file
  JobContent(remus::common::ContentSource::Type source,
             remus::common::ContentFormat::Type format,
             const std::string& contents);

  //pass in some Memory data to send to the worker. A pointer
  //to the contents is kept and no copy will of data will happen, so
  //the contents of the pointer can't be deleted while the JobContent instance
  //is valid.
  JobContent(remus::common::ContentFormat::Type format,
             const char* contents,
             std::size_t size);

  //returns if the source of the content is memory or a file
  remus::common::ContentSource::Type sourceType() const
    { return this->SourceType; }

  //get the storage format that we currently have setup for the source
  remus::common::ContentFormat::Type formatType() const
    { return this->FormatType; }

  //tag this section of data with a user defined tag
  void tag(const std::string& tag_value) { this->Tag = tag_value; }
  //get the value of the tag for this data
  const std::string& tag() const { return this->Tag; }

  const char* data() const;
  std::size_t dataSize() const;

  ///implement a less than operator and equal operator so you
  //can use the class in containers and algorithms
  bool operator<(const JobContent& other) const;
  bool operator==(const JobContent& other) const;

  friend std::ostream& operator<<(std::ostream &os, const JobContent &content)
    { content.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is, JobContent &content)
    { content = JobContent(is); return is; }

private:
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobContent(std::istream& buffer);


  remus::common::ContentSource::Type SourceType;
  remus::common::ContentFormat::Type FormatType;
  std::string Tag;

  struct InternalImpl;
  boost::shared_ptr<InternalImpl> Implementation;
};

//------------------------------------------------------------------------------
inline remus::proto::JobContent make_FileJobContent(
      const std::string& path,
      remus::common::ContentFormat::Type format = remus::common::ContentFormat::User)
{
  return remus::proto::JobContent(remus::common::ContentSource::File,
                                   format,
                                   path);
}

//------------------------------------------------------------------------------
inline remus::proto::JobContent make_MemoryJobContent(
      const std::string& content,
      remus::common::ContentFormat::Type format = remus::common::ContentFormat::User)
{
  return remus::proto::JobContent(remus::common::ContentSource::Memory,
                                   format,
                                   content);
}

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobContent& content)
{
  std::ostringstream buffer;
  buffer << content;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::proto::JobContent to_JobContent(const std::string& msg)
{
  std::istringstream buffer(msg);
  remus::proto::JobContent content;
  buffer >> content;
  return content;
}

//------------------------------------------------------------------------------
inline remus::proto::JobContent to_JobContent(const char* data, int size)
{
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobContent( temp );
}


}
}

#endif
