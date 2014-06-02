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

#include <remus/proto/JobResult.h>

#include <remus/proto/conversionHelpers.h>

//suppress warnings inside boost headers for gcc and clang
#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <boost/uuid/uuid_io.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <iostream>

namespace remus {
namespace proto {

//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid):
  JobId(jid),
  FormatType(),
  Data()
{
}

//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const remus::common::FileHandle& fileHandle):
  JobId(jid),
  FormatType(format),
  Data(fileHandle.path())
{
}


//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const std::string& contents):
  JobId(jid),
  FormatType(format),
  Data(contents)
{
}

//------------------------------------------------------------------------------
bool JobResult::operator<(const JobResult& other) const
{
  return this->id() < other.id();
}

//------------------------------------------------------------------------------
bool JobResult::operator==(const JobResult& other) const
{
  return this->id() == other.id();
}

//------------------------------------------------------------------------------
void JobResult::serialize(std::ostream& buffer) const
{
  buffer << this->id() << std::endl;
  buffer << this->formatType() << std::endl;
  buffer << this->Data.size() << std::endl;
  remus::internal::writeString(buffer,this->data());
}

//------------------------------------------------------------------------------
JobResult::JobResult(std::istream& buffer)
{
  int ftype=0;
  std::size_t dataSize=0;

  buffer >> this->JobId;
  buffer >> ftype;
  buffer >> dataSize;
  this->Data = remus::internal::extractString(buffer,dataSize);
  this->FormatType = static_cast<remus::common::ContentFormat::Type>(ftype);
}

}
}