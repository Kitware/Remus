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
#ifndef remus_client_JobMeshRequirements_h
#define remus_client_JobMeshRequirements_h

#include <string>
#include <sstream>

//for ContentFomrat and ContentSource
#include <remus/client/ContentTypes.h>
#include <remus/common/MeshIOType.h>
#include <remus/common/ConditionalStorage.h>

#include <remus/client/ClientExports.h>

namespace remus{
namespace client{

class REMUSCLIENT_EXPORT JobMeshRequirements
{
public:
  //returns if the source of the mesh requirements is memory or a file
  //this can allow people to pass just file paths for local workers
  ContentSource::Type sourceType() const { return this->SourceType; }

  //get the storage format that we currently have setup for the
  //mesh requirements. This is something like XML, or JSON.
  ContentFormat::Type formatType() const { return this->FormatType; }

  //get the mesh types for the given workers job mesh requirements
  const remus::common::MeshIOType& jobMeshTypes() const { return MeshType; }

  //get the name of the worker that this job mesh requirements is for
  const std::string& workerName() const { return this->WorkerName; }

  //get a worker specified tag that holds meta data information
  //about this job mesh requirements
  const std::string& tag() const { return this->Tag; }


  bool hasRequirements() const { return Storage.size() > 0;}
  std::size_t requirementsSize() const { return Storage.size(); }
  const char* requirements() const { return Storage.data(); }

  //implement a less than operator so that can store
  //JobMeshRequirments in sets.
  bool operator<(const JobMeshRequirements& other) const;

private:
  friend std::string to_string(
                        const remus::client::JobMeshRequirements& content);
  friend remus::client::JobMeshRequirements to_JobMeshRequirements(
                        const std::string& msg);

  //serialize function
  void serialize(std::stringstream& buffer) const;

  //deserialize constructor function
  explicit JobMeshRequirements(std::stringstream& buffer);

  ContentSource::Type SourceType;
  ContentFormat::Type FormatType;
  remus::common::MeshIOType MeshType;
  std::string WorkerName;
  std::string Tag;

  remus::common::ConditionalStorage Storage;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::client::JobMeshRequirements& reqs)
{
  std::stringstream buffer;
  reqs.serialize(buffer);
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::client::JobMeshRequirements
to_JobMeshRequirements(const std::string& msg)
{
  std::stringstream buffer(msg);
  return remus::client::JobMeshRequirements(buffer);
}

//------------------------------------------------------------------------------
inline remus::client::JobMeshRequirements
to_JobMeshRequirements(const char* data, int size)
{
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobMeshRequirements( temp );
}


}
}

#endif
