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

#include <remus/proto/JobRequirements.h>

#include <remus/common/ConditionalStorage.h>
#include <remus/common/ConversionHelper.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/make_shared.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <sstream>

namespace remus{
namespace proto{

struct JobRequirements::InternalImpl
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
JobRequirements::JobRequirements():
  SourceType(),
  FormatType(),
  MeshType(),
  WorkerName(),
  Tag(),
  Implementation(boost::make_shared<InternalImpl>(
                 static_cast<char*>(NULL),std::size_t(0)))
{
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const remus::common::FileHandle& reqs_file ):
  SourceType(remus::common::ContentSource::File),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(),
  Implementation(boost::make_shared<InternalImpl>(reqs_file))
{
}


//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const std::string& reqs ):
  SourceType(remus::common::ContentSource::Memory),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(),
  Implementation(boost::make_shared<InternalImpl>(reqs))
{
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const char* reqs,
                                 std::size_t reqs_size ):
  SourceType(remus::common::ContentSource::Memory),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(),
  Implementation(boost::make_shared<InternalImpl>(reqs,reqs_size))
{
}


//------------------------------------------------------------------------------
bool JobRequirements::hasRequirements() const
{
  return this->Implementation->size() > 0;
}

//------------------------------------------------------------------------------
std::size_t JobRequirements::requirementsSize() const
{
  return this->Implementation->size();
}

//------------------------------------------------------------------------------
const char* JobRequirements::requirements() const
{
  return this->Implementation->data();
}

//------------------------------------------------------------------------------
bool JobRequirements::operator<(const JobRequirements& other) const
{
  //the sort order as follows.
  //first comes mesh input & output type, this allows us to group all
  //requirements for a given input & output type together
  //than comes source type, allowing all File sources to come before
  //all Memory sources
  //than comes format type, allowing all user defined to come before
  //XML, JSON and BSOn
  //than comes worker name
  //than comes tag
  if ( !(this->meshTypes() == other.meshTypes()) )
  { return (this->meshTypes() < other.meshTypes()); }

  else if (this->sourceType() != other.sourceType())
  { return (this->sourceType() < other.sourceType()); }

  else if (this->formatType() != other.formatType())
  { return (this->formatType() < other.formatType()); }

  else if (this->workerName() != other.workerName())
  { return (this->workerName() < other.workerName()); }

  else if (this->tag() != other.tag())
  { return (this->tag() < other.tag()); }

 return false; //both objects are equal
}

//------------------------------------------------------------------------------
bool JobRequirements::operator==(const JobRequirements& other) const
{
  return ((this->meshTypes() == other.meshTypes()) &&
          (this->sourceType() == other.sourceType()) &&
          (this->formatType() == other.formatType()) &&
          (this->workerName() == other.workerName()) &&
          (this->tag() == other.tag()));
}

//------------------------------------------------------------------------------
bool JobRequirements::operator!=(const JobRequirements& other) const
{
  return !(*this == other);
}

//------------------------------------------------------------------------------
void JobRequirements::serialize(std::ostream& buffer) const
{
  buffer << this->sourceType() << std::endl;
  buffer << this->formatType() << std::endl;
  buffer << this->meshTypes() << std::endl;

  buffer << this->workerName().size() << std::endl;
  remus::internal::writeString( buffer, this->workerName());

  buffer << this->tag().size() << std::endl;
  remus::internal::writeString( buffer, this->tag());

  buffer << this->requirementsSize() << std::endl;
  remus::internal::writeString( buffer,
                                this->requirements(),
                                this->requirementsSize() );
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(std::istream& buffer)
{
  int stype=0, ftype=0, workerNameSize=0, tagSize=0, contentsSize=0;

  //read in the source and format types
  buffer >> stype;
  buffer >> ftype;
  buffer >> this->MeshType;

  this->SourceType = static_cast<remus::common::ContentSource::Type>(stype);
  this->FormatType = static_cast<remus::common::ContentFormat::Type>(ftype);

  buffer >> workerNameSize;
  this->WorkerName = remus::internal::extractString(buffer,workerNameSize);

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
std::string to_string(const remus::proto::JobRequirements& reqs)
{
  std::ostringstream buffer;
  buffer << reqs;
  return buffer.str();
}

//------------------------------------------------------------------------------
remus::proto::JobRequirements to_JobRequirements(const char* data, std::size_t size)
{
  std::stringstream buffer;
  remus::internal::writeString(buffer, data, size);
  remus::proto::JobRequirements reqs;
  buffer >> reqs;
  return reqs;
}

//------------------------------------------------------------------------------
JobRequirementsSet::JobRequirementsSet():
Container()
{
}

//------------------------------------------------------------------------------
JobRequirementsSet::JobRequirementsSet(const ContainerType& container):
Container(container)
{
}

//------------------------------------------------------------------------------
 bool JobRequirementsSet::operator==(const JobRequirementsSet& other) const
 {
  return (this->Container == other.Container);
 }

//------------------------------------------------------------------------------
 bool JobRequirementsSet::operator!=(const JobRequirementsSet& other) const
 {
  return !(this->Container == other.Container);
 }

//------------------------------------------------------------------------------
void JobRequirementsSet::serialize(std::ostream& buffer) const
{ //note don't use std::endl as it flushes stream and decrease performance
  buffer << this->Container.size() << std::endl;
  typedef JobRequirementsSet::ContainerType::const_iterator IteratorType;
  for(IteratorType i = this->Container.begin();
      i != this->Container.end(); ++i)
    {
    buffer << *i << '\n';
    }
}

//------------------------------------------------------------------------------
JobRequirementsSet::JobRequirementsSet(std::istream& buffer)
{
  std::size_t csize = 0;
  buffer >> csize;
  for(std::size_t i = 0; i < csize; ++i)
    {
    JobRequirements reqs;
    buffer >> reqs;
    this->Container.insert(reqs);
    }
}


}
}
