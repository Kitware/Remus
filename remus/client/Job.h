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

#ifndef remus_client_Job_h
#define remus_client_Job_h

#include <algorithm>
#include <string>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <remus/common/MeshIOType.h>

//The remus::client::Job class
// For the client side interface it holds the Id and Type of a submitted job.
// The client interface is given a job object after submitting a JobRequest.
// For the client this class holds no information on the details of the
// submitted job. But holds just enough information to ask the server for
// the status and results.

namespace remus{
namespace client{
class Job
{
public:

  //construct a valid client side job object with an Id and Type
  Job(const boost::uuids::uuid& id,
      const remus::common::MeshIOType& type):
  Id(id),
  Type(type)
  {
  }

  //get if the current job is a valid job
  bool valid() const { return Type.valid(); }

  //get the id of the job
  const boost::uuids::uuid& id() const { return Id; }

  //get the mesh type of the job
  const remus::common::MeshIOType& type() const { return Type; }

private:
  boost::uuids::uuid Id;
  remus::common::MeshIOType Type;
};

//------------------------------------------------------------------------------
inline std::string to_string(const remus::client::Job& job)
{
  //convert a job to a string, used as a hack to serialize
  //encoding is simple, contents newline separated
  std::stringstream buffer;
  buffer << job.type() << std::endl;
  buffer << job.id() << std::endl;
  return buffer.str();
}


//------------------------------------------------------------------------------
inline remus::client::Job to_Job(const std::string& msg)
{
  //convert a job detail from a string, used as a hack to serialize
  std::stringstream buffer(msg);

  remus::common::MeshIOType type;
  boost::uuids::uuid id;

  buffer >> type;
  buffer >> id;
  return remus::client::Job(id,type);
}


//------------------------------------------------------------------------------
inline remus::client::Job to_Job(const char* data, int size)
{
  //convert a job from a string, used as a hack to serialize
  std::string temp(size,char());
  std::copy( data, data+size, temp.begin() );
  return to_Job( temp );
}

}
}

#endif
