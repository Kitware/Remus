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

//for ContentFormat and ContentSource
#include <remus/client/ContentTypes.h>
#include <remus/common/MeshIOType.h>
#include <remus/common/ConditionalStorage.h>

#include <remus/client/ClientExports.h>

namespace remus{
namespace client{

class REMUSCLIENT_EXPORT JobMeshRequirements
{
public:
  //construct an invalid JobMeshRequirments. This constructor is designed
  //to allows this class to be stored in containers.
  JobMeshRequirements();

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

  //implement a less than operator and equal operator so you
  //can use the class in containers and algorithms
  bool operator<(const JobMeshRequirements& other) const;
  bool operator==(const JobMeshRequirements& other) const;

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobMeshRequirements &reqs)
    { reqs.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  JobMeshRequirements &reqs)
    { reqs = JobMeshRequirements(is); return is; }

private:
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobMeshRequirements(std::istream& buffer);

  ContentSource::Type SourceType;
  ContentFormat::Type FormatType;
  remus::common::MeshIOType MeshType;
  std::string WorkerName;
  std::string Tag;

  remus::common::ConditionalStorage Storage;
};

//a simple container so we can send a collection of requirements
//to and from the client easily.
struct REMUSCLIENT_EXPORT JobMeshRequirementsSet
{
  typedef std::set< JobMeshRequirements > ContainerType;

  JobMeshRequirementsSet();
  JobMeshRequirementsSet(const ContainerType& container);

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobMeshRequirementsSet &reqs)
    { reqs.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  JobMeshRequirementsSet &reqs)
    { reqs = JobMeshRequirementsSet(is); return is; }


  const ContainerType& get() const { return Container; }
  ContainerType& get() { return Container; }


private:
  void serialize(std::ostream& buffer) const;
  explicit JobMeshRequirementsSet(std::istream& buffer);

  ContainerType Container;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::client::JobMeshRequirements& reqs)
{
  std::ostringstream buffer;
  buffer << reqs;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::client::JobMeshRequirements
to_JobMeshRequirements(const std::string& msg)
{
  std::istringstream buffer(msg);
  remus::client::JobMeshRequirements reqs;
  buffer >> reqs;
  return reqs;
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
