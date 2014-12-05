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

#include <remus/proto/JobContent.h>

#include <remus/common/ConditionalStorage.h>
#include <remus/common/MD5Hash.h>
#include <remus/proto/conversionHelpers.h>

#include <boost/make_shared.hpp>

#include <sstream>
#include <algorithm>

namespace remus{
namespace proto{

struct JobContent::InternalImpl
{
  template<typename T>
  explicit InternalImpl(const T& t):
    Size(0),
    Data(NULL),
    Storage(),
    ShortHash(),
    FullHash()
  {
    remus::common::ConditionalStorage temp(t);
    this->Storage.swap(temp);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();
  }

  InternalImpl(const char* d, std::size_t s):
    Size(s),
    Data(d),
    Storage(),
    ShortHash(),
    FullHash()
  {
  }


  InternalImpl(const boost::shared_array<char> d, std::size_t s):
    Size(s),
    Data(NULL),
    Storage(),
    ShortHash(),
    FullHash()
{
    remus::common::ConditionalStorage temp(d,s);
    this->Storage.swap(temp);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();
  }

  std::size_t size() const { return Size; }
  const char* data() const { return Data; }

  bool equal(const boost::shared_ptr<InternalImpl> other)
    {
    return (this->shortHash() == other->shortHash()) &&
           (this->fullHash() == other->fullHash());
    }

  bool less(const boost::shared_ptr<InternalImpl> other)
    {
    if(this->shortHash() != other->shortHash())
      { return this->shortHash() < other->shortHash(); }
    if(this->fullHash() != other->fullHash())
      { return this->fullHash() < other->fullHash(); }
    return false;
    }

  const std::string& shortHash()
  {
    if(this->ShortHash.size() == 0 && this->size() < 4096)
      {
      this->ShortHash = std::string(this->data(),this->size());
      }
    else if(this->ShortHash.size() == 0)
      {
      //only hash the first 4096 characters
      std::size_t hashsize = 4096;
      this->ShortHash = remus::common::MD5Hash(this->data(),
                                               hashsize);
      }
    return this->ShortHash;
  }

  const std::string& fullHash()
  {
   if(this->FullHash.size() == 0)
      {
      //has the whole damn file
      this->FullHash = remus::common::MD5Hash(this->data(),
                                              this->size());
      }
    return this->FullHash;
  }
private:

  //store the size of the data being held
  std::size_t Size;

  //points to the zero copy or data in the conditional storage
  const char* Data;

  //Storage is an optional allocation that is used when we need to copy data
  remus::common::ConditionalStorage Storage;

  //MD5Hash of the data held by us.
  std::string ShortHash;
  std::string FullHash;
};

//------------------------------------------------------------------------------
JobContent::JobContent():
  SourceType(),
  FormatType(),
  Tag(),
  Implementation( boost::make_shared<InternalImpl>(
                 static_cast<char*>(NULL),std::size_t(0)) )
  //make_shared is significantly faster than using manual new
{
}

//------------------------------------------------------------------------------
JobContent::JobContent(remus::common::ContentFormat::Type format,
                       const remus::common::FileHandle& handle):
  SourceType(remus::common::ContentSource::File),
  FormatType(format),
  Tag(),
  Implementation( boost::make_shared<InternalImpl>(handle) )
  //make_shared is significantly faster than using manual new
{

}


//------------------------------------------------------------------------------
JobContent::JobContent(remus::common::ContentFormat::Type format,
                       const std::string& contents):
  SourceType(remus::common::ContentSource::Memory),
  FormatType(format),
  Tag(),
  Implementation( boost::make_shared<InternalImpl>(contents) )
  //make_shared is significantly faster than using manual new
{

}

//------------------------------------------------------------------------------
JobContent::JobContent(remus::common::ContentFormat::Type format,
                       const char* contents,
                       std::size_t size):
  SourceType(remus::common::ContentSource::Memory),
  FormatType(format),
  Tag(),
  Implementation( boost::make_shared<InternalImpl>(contents,size) )
  //make_shared is significantly faster than using manual new
{

}

//------------------------------------------------------------------------------
const char* JobContent::data() const
{
  return this->Implementation->data();
}

//------------------------------------------------------------------------------
std::size_t JobContent::dataSize() const
{
  return this->Implementation->size();
}

//------------------------------------------------------------------------------
bool JobContent::operator<(const JobContent& other) const
{
  if (this->sourceType() != other.sourceType())
  { return (this->sourceType() < other.sourceType()); }

  if (this->formatType() != other.formatType())
  { return (this->formatType() < other.formatType()); }

  if (this->tag() != other.tag())
  { return (this->tag() < other.tag()); }

  if (this->dataSize() != other.dataSize())
  { return (this->dataSize() < other.dataSize()); }

  //instead of comparing the full data of the content, we just compare
  //cached md5 hashes of the content
  if (!(this->Implementation->equal(other.Implementation)))
    { return (this->Implementation->less(other.Implementation)); }

  return false; //they are the same
}

//------------------------------------------------------------------------------
bool JobContent::operator==(const JobContent& other) const
{
  return (this->sourceType() == other.sourceType())
         && (this->formatType() == other.formatType())
         && (this->tag() == other.tag())
         && (this->dataSize() == other.dataSize())
         && (this->Implementation->equal(other.Implementation));
}

//------------------------------------------------------------------------------
void JobContent::serialize(std::ostream& buffer) const
{
  buffer << this->sourceType() << std::endl;
  buffer << this->formatType() << std::endl;

  buffer << this->tag().size() << std::endl;
  remus::internal::writeString(buffer,this->tag());

  buffer << this->Implementation->size() << std::endl;
  remus::internal::writeString( buffer,
                                this->Implementation->data(),
                                this->Implementation->size() );
}

//------------------------------------------------------------------------------
JobContent::JobContent(std::istream& buffer)
{
  int stype=0, ftype=0;
  std::size_t tagSize=0;
  std::size_t contentsSize=0;

  //read in the source and format types
  buffer >> stype;
  buffer >> ftype;
  this->SourceType = static_cast<remus::common::ContentSource::Type>(stype);
  this->FormatType = static_cast<remus::common::ContentFormat::Type>(ftype);

  //read in the tag data, if we have any
  buffer >> tagSize;
  this->Tag = remus::internal::extractString(buffer,tagSize);

  //read in the contents. By using a shared_array instead of a vector
  //we reduce the memory overhead, as that shared_array is used by
  //the conditional storage. So the net result is instead of having
  //3 copies of contents, we now have 2 ( conditional storage, and buffer )
  buffer >> contentsSize;
  boost::shared_array<char> contents( new char[contentsSize] );
  remus::internal::extractArray(buffer, contents.get(), contentsSize);

  //if we have read nothing in, and the array is empty, we need to explicitly
  //act like we have a null pointer, which doesn't happen if we pass in
  //contents as it has a non NULL location ( see spec 5.3.4/7 )
  if( contentsSize == 0)
    { //make_shared is significantly faster than using manual new
    this->Implementation = boost::make_shared<InternalImpl>(
                                    static_cast<char*>(NULL),std::size_t(0));
    }
  else
    { //make_shared is significantly faster than using manual new
    this->Implementation = boost::make_shared<InternalImpl>(
                                                contents, contentsSize);
    }
}

//------------------------------------------------------------------------------
std::string to_string(const remus::proto::JobContent& content)
{
  std::ostringstream buffer;
  buffer << content;
  return buffer.str();
}

//------------------------------------------------------------------------------
remus::proto::JobContent to_JobContent(const char* data, std::size_t size)
{
  std::stringstream buffer;
  remus::internal::writeString(buffer, data, size);
  remus::proto::JobContent content;
  buffer >> content;
  return content;
}


}
}
