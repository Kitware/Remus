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

#include <remus/common/ConversionHelper.h>

#include <algorithm>
#include <sstream>

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
  MeshType(reqs.meshTypes()),
  Requirements(reqs),
  Content()
{
}

//------------------------------------------------------------------------------
JobSubmission::JobSubmission( const remus::proto::JobRequirements& reqs,
                              const remus::proto::JobContent& content ):
  MeshType(reqs.meshTypes()),
  Requirements(reqs),
  Content( )
{
  this->Content[this->default_key()]=content;
}

//------------------------------------------------------------------------------
JobSubmission::JobSubmission( const remus::proto::JobRequirements& reqs,
            const std::map<std::string,remus::proto::JobContent>& content ):
  MeshType(reqs.meshTypes()),
  Requirements(reqs),
  Content(content)
{
}


//------------------------------------------------------------------------------
bool JobSubmission::operator<(const JobSubmission& other) const
{

  if ( !(this->type() == other.type()))
  { return (this->type() < other.type()); }

  if (!(this->requirements() == other.requirements()))
  { return (this->requirements() < other.requirements()); }

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
          (this->requirements() == other.requirements()) &&
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

//------------------------------------------------------------------------------
std::string to_string(const remus::proto::JobSubmission& sub)
{
  std::ostringstream buffer;
  buffer << sub;
  return buffer.str();
}

//------------------------------------------------------------------------------
remus::proto::JobSubmission to_JobSubmission(const char* data, std::size_t size)
{
  std::stringstream buffer;
  remus::internal::writeString(buffer, data, size);
  remus::proto::JobSubmission sub;
  buffer >> sub;
  return sub;
}



}
}
