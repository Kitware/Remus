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

#ifndef remus_proto_Message_h
#define remus_proto_Message_h

#include <boost/shared_ptr.hpp>

#include <remus/common/MeshIOType.h>
#include <remus/common/remusGlobals.h>

//for export symbols
#include <remus/proto/ProtoExports.h>

namespace zmq
{
  class message_t;
  class socket_t;
}

namespace remus{
namespace proto{

class REMUSPROTO_EXPORT Message
{
public:
  //----------------------------------------------------------------------------
  //pass in a std::string that Message will copy and send
  Message(remus::common::MeshIOType mtype,
          remus::SERVICE_TYPE stype,
          const std::string& data);

  //----------------------------------------------------------------------------
  //creates a Message with no data
  Message(remus::common::MeshIOType mtype,
          remus::SERVICE_TYPE stype);

  //----------------------------------------------------------------------------
  //creates a Message from reading from the socket
  explicit Message(zmq::socket_t* socket);

  //send a blocking message
  bool send(zmq::socket_t* socket) const;

  //send the message as non blocking so that we can do
  //async sending
  bool sendNonBlocking(zmq::socket_t* socket) const;

  const remus::common::MeshIOType& MeshIOType() const { return MType; }
  const remus::SERVICE_TYPE& serviceType() const { return SType; }

  const char* data() const;
  std::size_t dataSize() const;

  bool isValid() const { return ValidMsg; }

private:
  bool send_impl(zmq::socket_t* socket, int flags = 0) const;

  remus::common::MeshIOType MType;
  remus::SERVICE_TYPE SType;
  bool ValidMsg; //tells if the message is valid

  boost::shared_ptr<zmq::message_t> Storage;
};

}
}
#endif //remus_Message_h
