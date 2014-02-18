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

#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

//for ContentFormat and ContentSource
#include <remus/common/ContentTypes.h>
#include <remus/common/MeshIOType.h>

//for export symbols
#include <remus/proto/ProtoExports.h>

namespace remus{
namespace proto{

class REMUSPROTO_EXPORT JobRequirements
{
public:
  //construct an invalid JobRequirements. This constructor is designed
  //to allows this class to be stored in containers.
  JobRequirements();

  //Make a valid JobRequirements which creates a copy of the of the requirements
  //data and stores it.
  JobRequirements(remus::common::ContentSource::Type stype,
                  remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const std::string& reqs );

  //Make a valid JobRequirements which holds a pointer to the requirements data
  //and will not copy or delete the data, which means the calling code must
  //manage the lifespan of the data. This is best used when you have really
  //large memory footprint for you requirements and want a zero copy interface
  //to it.
  JobRequirements(remus::common::ContentSource::Type stype,
                  remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const char* reqs,
                  std::size_t reqs_size );

  //Make a valid JobRequirements which creates a copy of the of the requirements
  //data and stores it.
  JobRequirements(remus::common::ContentSource::Type stype,
                  remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const std::string& tag,
                  const std::string& reqs );

  //Make a valid JobRequirements which holds a pointer to the requirements data
  //and will not copy or delete the data, which means the calling code must
  //manage the lifespan of the data. This is best used when you have really
  //large memory footprint for you requirements and want a zero copy interface
  //to it.
  JobRequirements(remus::common::ContentSource::Type stype,
                  remus::common::ContentFormat::Type ftype,
                  remus::common::MeshIOType mtype,
                  const std::string& wname,
                  const std::string& tag,
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

  bool hasRequirements() const;
  std::size_t requirementsSize() const;
  const char* requirements() const;

  //implement a less than operator and equal operator so you
  //can use the class in containers and algorithms
  bool operator<(const JobRequirements& other) const;
  bool operator==(const JobRequirements& other) const;

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobRequirements &reqs)
    { reqs.serialize(os); return os; }

  friend std::istream& operator>>(std::istream &is,
                                  JobRequirements &reqs)
    { reqs = JobRequirements(is); return is; }

private:
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

private:
  void serialize(std::ostream& buffer) const;
  explicit JobRequirementsSet(std::istream& buffer);

  ContainerType Container;
};

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements make_FileJobRequirements(
      remus::common::ContentFormat::Type format,
      remus::common::MeshIOType meshtypes,
      const std::string& wname,
      const std::string& reqs)
{
  return remus::proto::JobRequirements(remus::common::ContentSource::File,
                                        format,
                                        meshtypes,
                                        wname,
                                        reqs);
}

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements make_MemoryJobRequirements(
      remus::common::ContentFormat::Type format,
      remus::common::MeshIOType meshtypes,
      const std::string& wname,
      const std::string& reqs )
{
  return remus::proto::JobRequirements(remus::common::ContentSource::Memory,
                                       format,
                                       meshtypes,
                                       wname,
                                       reqs);
}


//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobRequirements& reqs)
{
  std::ostringstream buffer;
  buffer << reqs;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements
to_JobRequirements(const std::string& msg)
{
  std::istringstream buffer(msg);
  remus::proto::JobRequirements reqs;
  buffer >> reqs;
  return reqs;
}

//------------------------------------------------------------------------------
inline remus::proto::JobRequirements
to_JobRequirements(const char* data, int size)
{
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobRequirements( temp );
}

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobRequirementsSet& reqs)
{
  std::ostringstream buffer;
  buffer << reqs;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::proto::JobRequirementsSet
to_JobRequirementsSet(const std::string& msg)
{
  std::istringstream buffer(msg);
  remus::proto::JobRequirementsSet reqs;
  buffer >> reqs;
  return reqs;
}

//------------------------------------------------------------------------------
inline remus::proto::JobRequirementsSet
to_JobRequirementsSet(const char* data, int size)
{
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_JobRequirementsSet( temp );
}


}
}

#endif
