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

#ifndef remus_client_JobContent_h
#define remus_client_JobContent_h

#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

//included for symbol exports
#include <remus/client/ClientExports.h>

namespace remus{
namespace client{

//forward declare for friendship
class Client;

//we need to work on these two more
struct ContentFormat{ enum Type{USER=0, XML=1, JSON=2, BSON=3}; };
struct ContentSource{ enum Type{File=0, Memory=1}; };

class REMUSCLIENT_EXPORT JobContent
{
public:
  //pass in some data to send to the worker. The
  //contents of the string will be copied into a local memory if the
  //data type is Memory. If the data type is file we will read the contents
  //of the file
  JobContent(ContentSource::Type source,
             ContentFormat::Type format,
             const std::string& contents);

  //pass in some Memory data to send to the worker. A pointer
  //to the contents is kept and no copy will of data will happen, so
  //the contents of the pointer can't be deleted while the JobContent instance
  //is valid.
  JobContent(ContentFormat::Type format,
             const char* contents,
             std::size_t size);

  //returns if the source of the content is memory or a file
  ContentSource::Type source_type() const { return this->SourceType; }

  //get the storage format that we currently have setup for the source
  ContentFormat::Type format_type() const { return this->FormatType; }

  //tag this section of data with a user defined tag
  void tag(const std::string& tag_value) { this->Tag = tag_value; }
  //get the value of the tag for this data
  const std::string& tag() const { return this->Tag; }

private:
  friend class remus::client::Client;
  friend std::string to_string(const remus::client::JobContent& content);
  friend remus::client::JobContent to_JobContent(const std::string& msg);

  //state to mark if the file should be read when we serialize
  void setServerToBeRemote(bool isRemote) const;

  //serialize function
  void serialize(std::stringstream& buffer) const;

  //deserialize function
  static remus::client::JobContent deserialize(std::stringstream& buffer);


  ContentSource::Type SourceType;
  ContentFormat::Type FormatType;
  std::string Tag;

  struct InternalImpl;
  boost::shared_ptr<InternalImpl> Implementation;
};

//------------------------------------------------------------------------------
inline remus::client::JobContent make_FileJobContent(
      remus::client::ContentFormat::Type format,
      const std::string& path)
{
  return remus::client::JobContent(remus::client::ContentSource::File,
                                   format,
                                   path);
}

//------------------------------------------------------------------------------
inline remus::client::JobContent make_MemoryJobContent(
      remus::client::ContentFormat::Type format,
      const std::string& content )
{
  return remus::client::JobContent(remus::client::ContentSource::Memory,
                                   format,
                                   content);
}

//------------------------------------------------------------------------------
inline std::string to_string(const remus::client::JobContent& content)
{
  std::stringstream buffer;
  content.serialize(buffer);
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::client::JobContent to_JobContent(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::stringstream buffer(msg);
  return remus::client::JobContent::deserialize(buffer);
}

//------------------------------------------------------------------------------
inline remus::client::JobContent to_JobContent(const char* data, int size)
{
  //convert a job request from a string, used as a hack to serialize
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobContent( temp );
}


}
}

#endif
