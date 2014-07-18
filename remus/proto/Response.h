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

#ifndef remus_proto_Response_h
#define remus_proto_Response_h

#include <boost/shared_ptr.hpp>

#include <remus/common/MeshIOType.h>
#include <remus/common/remusGlobals.h>
#include <remus/proto/zmqSocketIdentity.h>

//for export symbols
#include <remus/proto/ProtoExports.h>

namespace zmq
{
  class message_t;
  class socket_t;
}

namespace remus{
namespace proto{
class REMUSPROTO_EXPORT Response
{
public:
  //----------------------------------------------------------------------------
  //construct a response to send
  //The service type will be set to invalid.
  explicit Response();

  //----------------------------------------------------------------------------
  //construct a response to send, which has no data
  explicit Response(remus::common::SERVICE_TYPE);

  //----------------------------------------------------------------------------
  //construct a response, the contents of the string will copied and sent.
  Response(remus::common::SERVICE_TYPE, const std::string& data);

  //----------------------------------------------------------------------------
  //construct a response, the contents of the data pointer will copied and sent.
  Response(remus::common::SERVICE_TYPE, const char* data, int size);

  //----------------------------------------------------------------------------
  //create a response from reading from the socket
  explicit Response(zmq::socket_t* socket);

  remus::SERVICE_TYPE serviceType() const { return SType; }

  bool send(zmq::socket_t* socket, const zmq::SocketIdentity& client) const;

  bool sendNonBlocking(zmq::socket_t* socket, const zmq::SocketIdentity& client) const;

  const char* data() const;
  std::size_t dataSize() const;

  bool isValid() const { return ValidMsg; }

private:
  bool send_impl(zmq::socket_t* socket, int flags = 0) const;

  const zmq::SocketIdentity ClientAddress; //which client to send this response too
  remus::SERVICE_TYPE SType;
  bool ValidResponse; //tells if the response is valid

  boost::shared_ptr<zmq::message_t> Storage;

  //make copying not possible
  Response (const Response&);
  void operator = (const Response&);
};

}
}

#endif // remus_proto_Response_h
