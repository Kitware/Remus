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
#include <remus/proto/conversionHelpers.h>

namespace remus{
namespace proto{

struct JobRequirements::InternalImpl
{
  template<typename T>
  explicit InternalImpl(const T& t)
  {
    remus::common::ConditionalStorage temp(t);
    this->Storage.swap(temp);
    this->Size = this->Storage.size();
    this->Data = this->Storage.data();
  }

  InternalImpl(const char* data, std::size_t size):
    Size(size),
    Data(data),
    Storage()
  {
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
  Implementation(new InternalImpl(NULL,0))
{
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentSource::Type stype,
                                 remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const std::string& reqs ):
  SourceType(stype),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(),
  Implementation(new InternalImpl(reqs))
{
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentSource::Type stype,
                                 remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const char* reqs,
                                 std::size_t reqs_size ):
  SourceType(stype),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(),
  Implementation(new InternalImpl(reqs,reqs_size))
{
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentSource::Type stype,
                                 remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const std::string& tag,
                                 const std::string& reqs ):
  SourceType(stype),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(tag),
  Implementation(new InternalImpl(reqs))
{
}

//------------------------------------------------------------------------------
JobRequirements::JobRequirements(remus::common::ContentSource::Type stype,
                                 remus::common::ContentFormat::Type ftype,
                                 remus::common::MeshIOType mtype,
                                 const std::string& wname,
                                 const std::string& tag,
                                 const char* reqs,
                                 std::size_t reqs_size ):
  SourceType(stype),
  FormatType(ftype),
  MeshType(mtype),
  WorkerName(wname),
  Tag(tag),
  Implementation(new InternalImpl(reqs,reqs_size))
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
  return this->Implementation->size() > 0;
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
  if (this->jobMeshTypes().type() != other.jobMeshTypes().type())
  { return (this->jobMeshTypes().type() < other.jobMeshTypes().type()); }

  if (this->sourceType() != other.sourceType())
  { return (this->sourceType() < other.sourceType()); }

  if (this->formatType() != other.formatType())
  { return (this->formatType() < other.formatType()); }

  if (this->workerName() != other.workerName())
  { return (this->workerName() < other.workerName()); }

  if (this->tag() != other.tag())
  { return (this->tag() < other.tag()); }

 return false; //both objects are equal
}

//------------------------------------------------------------------------------
 bool JobRequirements::operator==(const JobRequirements& other) const
 {
  return ((this->jobMeshTypes().type() == other.jobMeshTypes().type()) &&
          (this->sourceType() == other.sourceType()) &&
          (this->formatType() == other.formatType()) &&
          (this->workerName() == other.workerName()) &&
          (this->tag() == other.tag()));
 }

//------------------------------------------------------------------------------
void JobRequirements::serialize(std::ostream& buffer) const
{
  buffer << this->sourceType() << std::endl;
  buffer << this->formatType() << std::endl;
  buffer << this->jobMeshTypes() << std::endl;

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

  //read in the contents, todo do this with less temp objects and copies
  buffer >> contentsSize;

  std::vector<char> contents(contentsSize);

  //enables us to use less copies for faster read of large data
  remus::internal::extractVector(buffer,contents);

  this->Implementation =
    boost::shared_ptr< InternalImpl >(new InternalImpl(contents));
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
void JobRequirementsSet::serialize(std::ostream& buffer) const
{
  buffer << this->Container.size() << std::endl;
  typedef JobRequirementsSet::ContainerType::const_iterator IteratorType;
  for(IteratorType i = this->Container.begin();
      i != this->Container.end(); ++i)
    {
    buffer << *i << std::endl;
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