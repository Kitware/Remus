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

#ifndef remus_proto_JobSubmission_h
#define remus_proto_JobSubmission_h

#include <string>
#include <map>

#include <remus/proto/JobContent.h>
#include <remus/proto/JobRequirements.h>

#include <remus/proto/ProtoExports.h>


namespace remus{
namespace proto{

class REMUSPROTO_EXPORT JobSubmission
{
public:
  typedef std::map<std::string,remus::proto::JobContent> ContainerType;
  typedef ContainerType::iterator iterator;
  typedef ContainerType::const_iterator const_iterator;
  typedef ContainerType::const_reference const_reference;
  typedef ContainerType::reference reference;
  typedef ContainerType::value_type value_type;

  //construct an invalid JobSubmission. This constructor is designed
  //to allows this class to be stored in containers.
  JobSubmission();

  JobSubmission( const remus::proto::JobRequirements& reqs );

  //A JobSubmission with only a single content object means that
  //we will copy and store the content as the following key value pair
  //{ key: 'default' , value=content }
  JobSubmission( const remus::proto::JobRequirements& reqs,
                 const remus::proto::JobContent& content);

  JobSubmission( const remus::proto::JobRequirements& reqs,
                 const ContainerType& content);

  //returns the default key for job content being inserted in the constructor
  //call
  std::string default_key( ) const { return "data"; }

  //add new content to the job submission. If the key is already in use
  //we will overwrite the job content with
  std::pair<iterator,bool> insert( const value_type& value )
    { return this->Content.insert(value); }

  template< class InputIt >
  void insert( InputIt first, InputIt last )
    { return this->Content.insert(first,last); }

  remus::proto::JobContent& operator[]( const std::string& key )
    { return this->Content[key]; }

  iterator begin() { return this->Content.begin(); }
  const_iterator begin() const { return this->Content.begin(); }

  iterator end( ) { return this->Content.end(); }
  const_iterator end( ) const { return this->Content.end(); }

  const_iterator find( const std::string& key ) const
    { return this->Content.find(key); }
  iterator find( const std::string& key )
    { return this->Content.find(key); }

  std::size_t size() const { return this->Content.size(); }

  //get the mesh types for this submission
  const remus::common::MeshIOType& type() const { return MeshType; }

  //get the requirements for this submission
  const remus::proto::JobRequirements& requirements( ) const
    { return this->Requirements; }

  //implement a less than operator and equal operator so you
  //can use the class in containers and algorithms
  bool operator<(const JobSubmission& other) const;
  bool operator==(const JobSubmission& other) const;

  friend std::ostream& operator<<(std::ostream &os,
                                  const JobSubmission &submission)
    { submission.serialize(os); return os; }

  //needed to decode the object from the wire
  friend std::istream& operator>>(std::istream &is,
                                  JobSubmission &submission)
    { submission = JobSubmission(is); return is; }

private:
  //serialize function
  void serialize(std::ostream& buffer) const;

  //deserialize constructor function
  explicit JobSubmission(std::istream& buffer);

  remus::common::MeshIOType MeshType;
  remus::proto::JobRequirements Requirements;
  ContainerType Content;


};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::proto::JobSubmission& sub)
{
  std::ostringstream buffer;
  buffer << sub;
  return buffer.str();
}

//------------------------------------------------------------------------------
inline remus::proto::JobSubmission
to_JobSubmission(const std::string& msg)
{
  remus::proto::JobSubmission submission;
  std::istringstream buffer(msg);
  buffer >> submission;
  return submission;
}

//------------------------------------------------------------------------------
inline remus::proto::JobSubmission
to_JobSubmission(const char* data, std::size_t length)
{
  std::string temp(data,length);
  return to_JobSubmission( temp );
}

}
}

#endif
