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

#include <remus/proto/SMTKMeshSubmission.h>
#include <remus/proto/JobContent.h>
#include <remus/proto/JobRequirements.h>

#include <iostream>

namespace {

//-----------------------------------------------------------------------------
void add_keyvalue(remus::proto::JobSubmission* submission,
                  const std::string& key,
                  const remus::proto::JobContent& value)
{
  submission->insert( std::pair<std::string,remus::proto::JobContent>(key,value) );
}

}
namespace remus{
namespace proto{

//-----------------------------------------------------------------------------
SMTKMeshSubmission::SMTKMeshSubmission():
  remus::proto::JobSubmission(),
  ModelKey("model"),
  AttributeKey("meshing_attributes"),
  ModelItemKey("modelUUIDS")
{

}

//-----------------------------------------------------------------------------
SMTKMeshSubmission::SMTKMeshSubmission( const remus::proto::JobRequirements& reqs ):
  remus::proto::JobSubmission( reqs ),
  ModelKey("model"),
  AttributeKey("meshing_attributes"),
  ModelItemKey("modelUUIDS")
{

}

//-----------------------------------------------------------------------------
SMTKMeshSubmission::SMTKMeshSubmission(const remus::proto::JobSubmission& submission):
  remus::proto::JobSubmission( submission ),
  ModelKey("model"),
  AttributeKey("meshing_attributes"),
  ModelItemKey("modelUUIDS")
{

}

//-----------------------------------------------------------------------------
bool SMTKMeshSubmission::hasAllComponents() const
{
  const bool has_model = this->find(this->ModelKey) != this->end();
  const bool has_attr = this->find(this->AttributeKey) != this->end();
  const bool has_mids = this->find(this->ModelItemKey) != this->end();
  return has_model && has_attr && has_mids;
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::model(const char* content, std::size_t len,
                               remus::common::ContentFormat::Type format)
{
  remus::proto::JobContent jc(format, content, len);
  add_keyvalue(this, this->ModelKey, jc);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::model(const std::string& content,
                               remus::common::ContentFormat::Type format)
{
  remus::proto::JobContent jc(format, content);
  add_keyvalue(this, this->ModelKey, jc);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::model(const remus::proto::JobContent& content)
{
  add_keyvalue(this, this->ModelKey, content);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::attributes(const char* content, std::size_t len,
                                    remus::common::ContentFormat::Type format)
{
  remus::proto::JobContent jc(format, content, len);
  add_keyvalue(this, this->AttributeKey, jc);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::attributes(const std::string& content,
                                    remus::common::ContentFormat::Type format)
{
  remus::proto::JobContent jc(format, content);
  add_keyvalue(this, this->AttributeKey, jc);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::attributes(const remus::proto::JobContent& content)
{
  add_keyvalue(this, this->AttributeKey, content);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::modelItemsToMesh(const char* content, std::size_t len,
                                          remus::common::ContentFormat::Type format)
{
  remus::proto::JobContent jc(format, content, len);
  add_keyvalue(this, this->ModelItemKey, jc);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::modelItemsToMesh(const std::string& content,
                                          remus::common::ContentFormat::Type format)
{
  remus::proto::JobContent jc(format, content);
  add_keyvalue(this, this->ModelItemKey, jc);
}

//-----------------------------------------------------------------------------
void SMTKMeshSubmission::modelItemsToMesh(const remus::proto::JobContent& content)
{
  add_keyvalue(this, this->ModelItemKey, content);
}

//-----------------------------------------------------------------------------
remus::proto::JobContent SMTKMeshSubmission::model() const
{
  typedef remus::proto::JobSubmission::const_iterator it;
  it item = this->find(this->ModelKey);
  if(item == this->end())
    {
    return remus::proto::JobContent();
    }
  return item->second;
}

//-----------------------------------------------------------------------------
remus::proto::JobContent SMTKMeshSubmission::attributes() const
{
  typedef remus::proto::JobSubmission::const_iterator it;
  it item = this->find(this->AttributeKey);
  if(item == this->end())
    {
    return remus::proto::JobContent();
    }
  return item->second;
}

//-----------------------------------------------------------------------------
remus::proto::JobContent SMTKMeshSubmission::modelItemsToMesh() const
{
  typedef remus::proto::JobSubmission::const_iterator it;
  it item = this->find(this->ModelItemKey);
  if(item == this->end())
    {
    return remus::proto::JobContent();
    }
  return item->second;
}

//------------------------------------------------------------------------------
SMTKMeshSubmission::SMTKMeshSubmission(std::istream& buffer):
  remus::proto::JobSubmission(buffer),
  ModelKey("model"),
  AttributeKey("meshing_attributes"),
  ModelItemKey("modelUUIDS")
{
}

//------------------------------------------------------------------------------
remus::proto::SMTKMeshSubmission to_SMTKMeshSubmission(const char* data, std::size_t size)
{
  remus::proto::JobSubmission js = remus::proto::to_JobSubmission(data, size);
  return remus::proto::SMTKMeshSubmission(js);
}

}
}
