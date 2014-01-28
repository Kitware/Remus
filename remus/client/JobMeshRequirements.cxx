
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

#include <remus/client/JobMeshRequirements.h>
#include <remus/common/remusGlobals.h>

namespace remus{
namespace client{

//------------------------------------------------------------------------------
 bool JobMeshRequirements::operator<(const JobMeshRequirements& other) const
{
 return true;
}

//------------------------------------------------------------------------------
void JobMeshRequirements::serialize(std::stringstream& buffer) const
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
JobMeshRequirements::JobMeshRequirements(std::stringstream& buffer)
{
  int stype=0, ftype=0, workerNameSize=0, tagSize=0, contentsSize=0;

  //read in the source and format types
  buffer >> stype;
  buffer >> ftype;
  buffer >> this->MeshType;

  this->SourceType = static_cast<ContentSource::Type>(stype);
  this->FormatType = static_cast<ContentFormat::Type>(ftype);

  buffer >> workerNameSize;
  this->WorkerName = remus::internal::extractString(buffer,workerNameSize);

  buffer >> tagSize;
  this->Tag = remus::internal::extractString(buffer,tagSize);

  //read in the contents, todo do this with less temp objects and copies
  buffer >> contentsSize;
  this->Storage = remus::common::ConditionalStorage(
                      remus::internal::extractString(buffer,contentsSize) );
}


}
}
