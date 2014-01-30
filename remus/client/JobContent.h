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

//for ContentFormat and ContentSource
#include <remus/client/ContentTypes.h>

//included for symbol exports
#include <remus/client/ClientExports.h>

namespace remus{
namespace client{

//forward declare for friendship
class Client;

class REMUSCLIENT_EXPORT JobContent
{
public:
  //construct an invalid JobContent. This constructor is designed
  //to allows this class to be stored in containers.
  JobContent();

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
  ContentSource::Type sourceType() const { return this->SourceType; }

  //get the storage format that we currently have setup for the source
  ContentFormat::Type formatType() const { return this->FormatType; }

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

private:
  friend class remus::client::Client;
  friend std::string to_string(const remus::client::JobContent& content);
  friend remus::client::JobContent to_JobContent(const std::string& msg);

  //get the md5hash of the data stored by the job content.
  std::string hash() const;

  //state to mark if the file should be read when we serialize
  void setServerToBeRemote(bool isRemote) const;

  //serialize function
  void serialize(std::stringstream& buffer) const;

  //deserialize constructor function
  explicit JobContent(std::stringstream& buffer);


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
  std::stringstream buffer(msg);
  return remus::client::JobContent(buffer);
}

//------------------------------------------------------------------------------
inline remus::client::JobContent to_JobContent(const char* data, int size)
{
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobContent( temp );
}


}
}

#endif
