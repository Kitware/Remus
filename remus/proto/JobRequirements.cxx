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
#include <remus/common/remusGlobals.h>

namespace remus{
namespace proto{

//------------------------------------------------------------------------------
JobRequirements::JobRequirements():
  SourceType(),
  FormatType(),
  MeshType(),
  WorkerName(),
  Tag(),
  Storage()
{
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
  this->Storage = remus::common::ConditionalStorage(
                      remus::internal::extractString(buffer,contentsSize) );
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