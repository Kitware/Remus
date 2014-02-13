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

#include <remus/proto/JobSubmission.h>

#include <remus/proto/conversionHelpers.h>

#include <algorithm>

namespace remus{
namespace proto{

//------------------------------------------------------------------------------
JobSubmission::JobSubmission(  ):
  MeshType( ),
  Requirements( ),
  Content()
{
}

//------------------------------------------------------------------------------
JobSubmission::JobSubmission( const remus::proto::JobRequirements& reqs ):
  MeshType(reqs.jobMeshTypes()),
  Requirements(reqs),
  Content()
{
}

//------------------------------------------------------------------------------
JobSubmission::JobSubmission( const remus::proto::JobRequirements& reqs,
                              const remus::proto::JobContent& content ):
  MeshType(reqs.jobMeshTypes()),
  Requirements(reqs),
  Content( )
{
  this->Content["default"]=content;
}

//------------------------------------------------------------------------------
JobSubmission::JobSubmission( const remus::proto::JobRequirements& reqs,
            const std::map<std::string,remus::proto::JobContent>& content ):
  MeshType(reqs.jobMeshTypes()),
  Requirements(reqs),
  Content(content)
{
}


//------------------------------------------------------------------------------
bool JobSubmission::operator<(const JobSubmission& other) const
{

  if ( !(this->type() == other.type()))
  { return (this->type() < other.type()); }

  if (!(this->retrieveRequirements() == other.retrieveRequirements()))
  { return (this->retrieveRequirements() < other.retrieveRequirements()); }

  if (this->Content.size() != other.Content.size())
  {  return this->Content < other.Content; }

 return false; //both objects are equal
}

//------------------------------------------------------------------------------
bool JobSubmission::operator==(const JobSubmission& other) const
 {
  //we need to think about this more, a job submission isn't really unique
  //if we don't look at the submitted job content. You can submit the same
  return ((this->type() == other.type()) &&
          (this->retrieveRequirements() == other.retrieveRequirements()) &&
          (this->Content.size() == other.Content.size()) &&
          (this->Content == other.Content)
          );
 }

//------------------------------------------------------------------------------
void JobSubmission::serialize(std::ostream& buffer) const
{
  buffer << this->MeshType << std::endl;
  buffer << this->Requirements << std::endl;
  buffer << this->Content.size() << std::endl;
  for(JobSubmission::const_iterator i = this->begin();
      i != this->end();
      ++i)
    {
    buffer << i->first.size() << std::endl;;
    remus::internal::writeString(buffer,i->first);
    buffer << i->second << std::endl;
    }
}

//------------------------------------------------------------------------------
JobSubmission::JobSubmission(std::istream& buffer)
{
  std::size_t contentSize=0;
  buffer >> this->MeshType;
  buffer >> this->Requirements;
  buffer >> contentSize;
  for(std::size_t i = 0; i < contentSize; ++i)
    {
    std::size_t keySize=0;
    buffer >> keySize;

    std::string key = remus::internal::extractString(buffer,keySize);

    JobContent value;
    buffer >> value;
    this->Content[key]=value;
    }
}


}
}
