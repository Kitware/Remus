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
  //construct a response, the contents of the string will copied and sent.
  Response(remus::SERVICE_TYPE stype, const std::string& data);

  //----------------------------------------------------------------------------
  //construct a response, the contents of the data pointer will copied and sent.
  Response(remus::SERVICE_TYPE stype, const char* data,
           std::size_t size);

  //----------------------------------------------------------------------------
  //create a response from reading from the socket
  explicit Response(zmq::socket_t* socket);

  remus::SERVICE_TYPE serviceType() const { return SType; }

  //returns true if the service for this response is a valid service
  bool isValidService() const { return SType != remus::INVALID_SERVICE; }

  bool send(zmq::socket_t* socket, const zmq::SocketIdentity& client) const;

  bool sendNonBlocking(zmq::socket_t* socket, const zmq::SocketIdentity& client) const;

  //the data should never be NULL, but could point to a filler string
  //you should always check isFullyFormed and isValidService before
  //trusting the data() and dataSize() methods
  const char* data() const;
  std::size_t dataSize() const;

  //Returns true if the message has all its contents. This only
  //makes sense when we construct a Response from a socket and want
  //to make sure we have everything
  bool isFullyFormed() const { return FullyFormed; }

private:
  bool send_impl(zmq::socket_t* socket,  const zmq::SocketIdentity& client,
                 int flags = 0) const;

  remus::SERVICE_TYPE SType;
  bool FullyFormed;

  boost::shared_ptr<zmq::message_t> Storage;

  //make copying not possible
  Response (const Response&);
  void operator = (const Response&);
};

}
}

#endif // remus_proto_Response_h
