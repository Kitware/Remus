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

#include <remus/client/JobContent.h>
#include <remus/common/ConditionalStorage.h>
#include <remus/common/MD5Hash.h>
#include <remus/common/remusGlobals.h>

#include <cassert>

#include <iostream>

namespace remus{
namespace client{

struct JobContent::InternalImpl
{

  template<typename T>
  explicit InternalImpl(const T& t)
  {
    this->Storage = remus::common::ConditionalStorage(t);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();

    //JobContent can store really large data, so in the future
    //we need to move to a smarter hashing strategy. I think
    //something where we hash only the first 1024bytes on creation
    //and than hash the full data lazily when we are compared to the
    //first item that matches our short hash is the best way.
    this->Hash = remus::common::MD5Hash(this->Storage);

    this->ServerIsRemote = false;
  }

  InternalImpl(const char* data, std::size_t size):
    Size(size),
    Data(data),
    Storage(),
    Hash(remus::common::MD5Hash(data,size)),
    ServerIsRemote(false)
  {
  }

  //store the size of the data being held
  int Size;
  //points to the zero copy or data in the conditional storage
  const char* Data;
  //Storage is an optional allocation that is used when we need to copy data
  remus::common::ConditionalStorage Storage;

  //MD5Hash of the data held by us.
  std::string Hash;

  //is the server remote or not
  bool ServerIsRemote;

};

//------------------------------------------------------------------------------
JobContent::JobContent():
  SourceType(),
  FormatType(),
  Tag(),
  Implementation( new InternalImpl(NULL,0) )
{
}

//------------------------------------------------------------------------------
JobContent::JobContent(ContentSource::Type source,
                       ContentFormat::Type format,
                       const std::string& contents):
  SourceType(source),
  FormatType(format),
  Tag(),
  Implementation( new InternalImpl(contents) )
{

}

//------------------------------------------------------------------------------
JobContent::JobContent(ContentFormat::Type format,
                       const char* contents,
                       std::size_t size):
  SourceType(ContentSource::Memory),
  FormatType(format),
  Tag(),
  Implementation( new InternalImpl(contents,size) )
{

}

//------------------------------------------------------------------------------
void JobContent::setServerToBeRemote(bool isRemote) const
{
  this->Implementation->ServerIsRemote = isRemote;
}

//------------------------------------------------------------------------------
const char* JobContent::data() const
{
  return this->Implementation->Data;
}

//------------------------------------------------------------------------------
std::size_t JobContent::dataSize() const
{
  return this->Implementation->Size;
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
  if (this->hash() != other.hash())
  { return this->hash() < other.hash(); }

  return false; //they are the same
}

//------------------------------------------------------------------------------
bool JobContent::operator==(const JobContent& other) const
{
  return (this->sourceType() == other.sourceType())
         && (this->formatType() == other.formatType())
         && (this->tag() == other.tag())
         && (this->dataSize() == other.dataSize())
         && (this->hash() == other.hash());
}

//------------------------------------------------------------------------------
std::string JobContent::hash() const
{
  return this->Implementation->Hash;
}

//------------------------------------------------------------------------------
void JobContent::serialize(std::stringstream& buffer) const
{
  buffer << this->sourceType() << std::endl;
  buffer << this->formatType() << std::endl;

  buffer << this->tag().size() << std::endl;
  remus::internal::writeString(buffer,this->tag());

  buffer << this->hash().size() << std::endl;
  remus::internal::writeString(buffer,this->hash());

  buffer << this->Implementation->Size << std::endl;
  remus::internal::writeString( buffer,
                                this->Implementation->Data,
                                this->Implementation->Size );


}

//------------------------------------------------------------------------------
JobContent::JobContent(std::stringstream& buffer)
{
  int stype=0, ftype=0, tagSize=0, hashSize=0,contentsSize=0;

  //read in the source and format types
  buffer >> stype;
  buffer >> ftype;
  this->SourceType = static_cast<ContentSource::Type>(stype);
  this->FormatType = static_cast<ContentFormat::Type>(ftype);

  //read in the tag data, if we have any
  buffer >> tagSize;
  this->Tag = remus::internal::extractString(buffer,tagSize);

  //read in the md5 has of the data
  buffer >> hashSize;
  std::string wire_hash = remus::internal::extractString(buffer,hashSize);

  //read in the contents, todo do this with less temp objects and copies
  buffer >> contentsSize;
  const std::string temp = remus::internal::extractString(buffer,contentsSize);
  this->Implementation = boost::shared_ptr< InternalImpl >(new InternalImpl(temp));

  //verify the hash we received is the same as the one we computed.
  assert( wire_hash == this->hash() );
}


}
}
