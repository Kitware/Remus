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

#include <remus/common/ConditionalStorage.h>
#include <remus/common/MD5Hash.h>
#include <remus/proto/conversionHelpers.h>

#include <boost/make_shared.hpp>

//suppress warnings inside boost headers for gcc, clang and MSVC
#ifndef _MSC_VER
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wshadow"
#else
# pragma warning(push)
//disable warning about using std::copy with pointers
# pragma warning(disable: 4996)
#endif
#include <boost/uuid/uuid_io.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#else
# pragma warning(pop)
#endif

#include <algorithm>
#include <sstream>

namespace remus {
namespace proto {


struct JobResult::InternalImpl
{
  template<typename T>
  explicit InternalImpl(const T& t):
    Size(0),
    Data(NULL),
    Storage()
  {
    remus::common::ConditionalStorage temp(t);
    this->Storage.swap(temp);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();
  }

  InternalImpl(const char* d, std::size_t s):
    Size(s),
    Data(d),
    Storage()
  {
  }

  InternalImpl(const boost::shared_array<char> d, std::size_t s):
    Size(s),
    Data(NULL),
    Storage()
  {
    remus::common::ConditionalStorage temp(d,s);
    this->Storage.swap(temp);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();
  }

  std::size_t size() const { return Size; }
  const char* data() const { return Data; }

private:

  //store the size of the data being held
  std::size_t Size;

  //points to the zero copy or data in the conditional storage
  const char* Data;

  //Storage is an optional allocation that is used when we need to copy data
  remus::common::ConditionalStorage Storage;
};

//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid):
  JobId(jid),
  FormatType(),
  Implementation( boost::make_shared<InternalImpl>(
                 static_cast<char*>(NULL),std::size_t(0)) )
  //make_shared is significantly faster than using manual new
{
}

//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const remus::common::FileHandle& fileHandle):
  JobId(jid),
  FormatType(format),
  Implementation( boost::make_shared<InternalImpl>(fileHandle) )
  //make_shared is significantly faster than using manual new
{
}


//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const std::string& contents):
  JobId(jid),
  FormatType(format),
  Implementation( boost::make_shared<InternalImpl>(contents) )
  //make_shared is significantly faster than using manual new
{
}

//------------------------------------------------------------------------------
JobResult::JobResult(const boost::uuids::uuid& jid,
            remus::common::ContentFormat::Type format,
            const char* contents,
            std::size_t size):
  JobId(jid),
  FormatType(format),
  Implementation( boost::make_shared<InternalImpl>(contents,size) )
  //make_shared is significantly faster than using manual new
{

}

//------------------------------------------------------------------------------
bool JobResult::valid() const
{
  return this->Implementation->size() != 0;
}

//------------------------------------------------------------------------------
const char* JobResult::data() const
{
  return this->Implementation->data();
}

//------------------------------------------------------------------------------
std::size_t JobResult::dataSize() const
{
  return this->Implementation->size();
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
  buffer << this->Implementation->size() << std::endl;
  remus::internal::writeString( buffer,
                                this->Implementation->data(),
                                this->Implementation->size() );
}

//------------------------------------------------------------------------------
JobResult::JobResult(std::istream& buffer)
{
  int ftype=0;
  std::size_t contentsSize=0;

  buffer >> this->JobId;
  buffer >> ftype;

  this->FormatType = static_cast<remus::common::ContentFormat::Type>(ftype);

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
std::string to_string(const remus::proto::JobResult& result)
{
  std::ostringstream buffer;
  buffer << result;
  return buffer.str();
}

//------------------------------------------------------------------------------
remus::proto::JobResult to_JobResult(const char* data, std::size_t size)
{
  std::stringstream buffer;
  remus::internal::writeString(buffer, data, size);
  remus::proto::JobResult res(buffer);
  return res;
}


}
}
