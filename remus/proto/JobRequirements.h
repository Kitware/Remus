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

#ifndef remus_proto_JobRequirements_h
#define remus_proto_JobRequirements_h

//for ContentFormat and ContentSource
#include <remus/common/ContentTypes.h>
#include <remus/common/FileHandle.h>
#include <remus/common/MeshIOType.h>

//for export symbols
#include <remus/proto/ProtoExports.h>
#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <string>
#include <set>

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif


namespace  remus { namespace worker { class Worker; } }

namespace remus{
namespace proto{

class REMUSPROTO_EXPORT JobRequirements
{
public:
  //construct an invalid JobRequirements. This constructor is designed
  //to allows this class to be stored in containers.
  JobRequirements();

  //Make a valid file source type JobRequirements which reads in the contents
  //of the file stream into the job requirement on construction
  // SourceType: File
  // FormatType: set by user
  JobRequirements(remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const remus::common::FileHandle& rfile );

  //Make a valid memory souce type JobRequirements which creates a copy of the
  //of the requirements data and stores it.
  // SourceType: Memory
  // FormatType: set by user
  JobRequirements(remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const std::string& reqs );

  //Make a valid JobRequirements which holds a pointer to the requirements data
  //and will not copy or delete the data, which means the calling code must
  //manage the lifespan of the data. This is best used when you have really
  //large memory footprint for you requirements and want a zero copy interface
  //to it.
  // SourceType: Memory
  // FormatType: set by user
  JobRequirements(remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const char* reqs,
                  std::size_t reqs_size );

  //returns if the source of the mesh requirements is memory or a file
  //this can allow people to pass just file paths for local workers
  remus::common::ContentSource::Type sourceType() const
    { return this->SourceType; }

  //get the storage format that we currently have setup for the
  //mesh requirements. This is something like XML, or JSON.
  remus::common::ContentFormat::Type formatType() const
    { return this->FormatType; }

  //get the mesh types for the given workers job mesh requirements
  const remus::common::MeshIOType& meshTypes() const { return MeshType; }

  //get the name of the worker that this job mesh requirements is for
  const std::string& workerName() const { return this->WorkerName; }

  //get a worker specified tag that holds meta data information
  //about this job mesh requirements
  const std::string& tag() const { return this->Tag; }
  void tag(const std::string& t) { this->Tag=t; }

  bool hasRequirements() const;
  std::size_t requirementsSize() const;

  //returns the raw requirements data stream. If the source type is file,
  //this will be the path to the file. In the future we plan to extend remus
  //to support automatic file reading.
  const char* requirements() const;

  //implement a less than operator and equal operator so you
  //can use the class in containers and algorithms
  bool operator<(const JobRequirements& other) const;
  bool operator==(const JobRequirements& other) const;
  bool operator!=(const JobRequirements& other) const;

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobRequirements &reqs)
    { reqs.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  JobRequirements &reqs)
    { reqs = JobRequirements(is); return is; }

private:
  //The worker needs to be a friend to send lightweight representations
  //of the full requirements to the server. This is the easiest way to do so.
  friend class remus::worker::Worker;


  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobRequirements(std::istream& buffer);

  remus::common::ContentSource::Type SourceType;
  remus::common::ContentFormat::Type FormatType;
  remus::common::MeshIOType MeshType;
  std::string WorkerName;
  std::string Tag;

  struct InternalImpl;
  boost::shared_ptr<InternalImpl> Implementation;
};

//a simple container so we can send a collection of requirements
//to and from the client easily.
struct REMUSPROTO_EXPORT JobRequirementsSet
{
  typedef std::set< JobRequirements > ContainerType;

  typedef ContainerType::iterator iterator;
  typedef ContainerType::const_iterator const_iterator;
  typedef ContainerType::reference reference;
  typedef ContainerType::const_reference const_reference;
  typedef ContainerType::value_type value_type;

  JobRequirementsSet();
  JobRequirementsSet(const ContainerType& container);

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobRequirementsSet &reqs)
    { reqs.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  JobRequirementsSet &reqs)
    { reqs = JobRequirementsSet(is); return is; }

  std::pair<iterator,bool> insert( const value_type& value )
    { return this->Container.insert(value); }

  template< class InputIt >
  void insert( InputIt first, InputIt last )
    { return this->Container.insert(first,last); }

  iterator begin() { return this->Container.begin(); }
  const_iterator begin() const { return this->Container.begin(); }

  iterator end( ) { return this->Container.end(); }
  const_iterator end( ) const { return this->Container.end(); }

  std::size_t size() const { return this->Container.size(); }

  std::size_t count( const value_type& item ) const
    { return this->Container.count(item); }

bool operator==(const JobRequirementsSet& other) const;
bool operator!=(const JobRequirementsSet& other) const;

private:
  void serialize(std::ostream& buffer) const;
  explicit JobRequirementsSet(std::istream& buffer);

  ContainerType Container;
};

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements make_JobRequirements(
      remus::common::MeshIOType meshtypes,
      const std::string& wname,
      const remus::common::FileHandle& rfile,
      remus::common::ContentFormat::Type format = remus::common::ContentFormat::User)
{
  return remus::proto::JobRequirements(format,
                                       meshtypes,
                                       wname,
                                       rfile);
}

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements make_JobRequirements(
      remus::common::MeshIOType meshtypes,
      const std::string& wname,
      const std::string& reqs,
      remus::common::ContentFormat::Type format = remus::common::ContentFormat::User)
{
  return remus::proto::JobRequirements(format,
                                       meshtypes,
                                       wname,
                                       reqs);
}

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT
std::string to_string(const remus::proto::JobRequirements& reqs);

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT
remus::proto::JobRequirements to_JobRequirements(const char* data, std::size_t size);

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements to_JobRequirements(const std::string& msg)
{
  return to_JobRequirements(msg.c_str(), msg.size());
}

}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif


#endif
