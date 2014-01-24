
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


namespace remus{
namespace client{

struct JobContent::InternalImpl
{

  template<typename T>
  InternalImpl(const T& t)
  {
    this->Storage = remus::common::ConditionalStorage(t);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();
    this->ServerIsRemote = false;
  }

  InternalImpl(const char* data, std::size_t size):
    Size(size),
    Data(data),
    Storage(),
    ServerIsRemote(false)
  { }

  //store the size of the data being held
  int Size;
  //points to the zero copy or data in the conditional storage
  const char* Data;
  //Storage is an optional allocation that is used when we need to copy data
  remus::common::ConditionalStorage Storage;
  //is the server remote or not
  bool ServerIsRemote;
};

//------------------------------------------------------------------------------
JobContent::JobContent(ContentSource::Type source,
                       ContentFormat::Type format,
                       const std::string& contents):
  SourceType(source),
  FormatType(format),
  Implementation( new InternalImpl(contents) )
{

}

//------------------------------------------------------------------------------
JobContent::JobContent(ContentFormat::Type format,
                       const char* contents,
                       std::size_t size):
  SourceType(ContentSource::Memory),
  FormatType(format),
  Implementation( new InternalImpl(contents,size) )
{

}

//------------------------------------------------------------------------------
void JobContent::setServerToBeRemote(bool isRemote) const
{
  this->Implementation->ServerIsRemote = isRemote;
}

//------------------------------------------------------------------------------
void JobContent::serialize(std::stringstream& buffer) const
{

}

//------------------------------------------------------------------------------
static remus::client::JobContent deserialize(std::stringstream& buffer)
{
  return remus::client::JobContent(ContentSource::File,
                                   ContentFormat::USER,
                                   std::string());
}


}
}
