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

#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <remus/common/MeshIOType.h>
#include <remus/common/ServiceTypes.h>
#include <remus/common/StatusTypes.h>
#include <remus/proto/zmqSocketIdentity.h>

//for export symbols
#include <remus/proto/ProtoExports.h>

#include <remus/common/CompilerInformation.h>
#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif

namespace zmq
{
  class message_t;
  class socket_t;
}

namespace remus{
namespace proto{

//forward declare response class;
class Response;

//----------------------------------------------------------------------------
//pass in a std::string that we will copy and send.
//The response returned will have a copy of the data given to it.
REMUSPROTO_EXPORT
Response send_Response(remus::SERVICE_TYPE stype,
                       const std::string& data,
                       zmq::socket_t* socket,
                       const zmq::SocketIdentity& client);

//----------------------------------------------------------------------------
//pass in a std::string that we will copy and send.
//The response returned will have a copy of the data given to it.
REMUSPROTO_EXPORT
Response send_NonBlockingResponse(remus::SERVICE_TYPE stype,
                                  const std::string& data,
                                  zmq::socket_t* socket,
                                  const zmq::SocketIdentity& client);

//----------------------------------------------------------------------------
//parse a response from a socket
//The response returned will have data associated with if it is valid
REMUSPROTO_EXPORT
Response receive_Response( zmq::socket_t* socket );

//----------------------------------------------------------------------------
//forward a response that has been received to another socket
//
REMUSPROTO_EXPORT
bool forward_Response(const remus::proto::Response& response,
                      zmq::socket_t* socket,
                      const zmq::SocketIdentity& client);

class REMUSPROTO_EXPORT Response
{
public:
  enum SendMode{Blocking=0, NonBlocking=1};

  const remus::SERVICE_TYPE& serviceType() const { return SType; }

  //the data should never be NULL, but could point to a filler string
  //you should always check isFullyFormed and isValidService before
  //trusting the data() and dataSize() methods
  const char* data() const;
  std::size_t dataSize() const;

  //is true if all the response was sent, or all of the response was received.
  bool isValid() const { return Valid; }

  Response(const Response&) = default;
  Response& operator=(Response&& other);
  Response& operator=(const Response&) = default;

private:

  friend REMUSPROTO_EXPORT Response send_Response(remus::SERVICE_TYPE stype,
                                                  const std::string& data,
                                                  zmq::socket_t* socket,
                                                  const zmq::SocketIdentity& client);


  friend REMUSPROTO_EXPORT Response send_NonBlockingResponse(remus::SERVICE_TYPE stype,
                                                             const std::string& data,
                                                             zmq::socket_t* socket,
                                                             const zmq::SocketIdentity& client);

  friend REMUSPROTO_EXPORT Response receive_Response( zmq::socket_t* socket );

  friend REMUSPROTO_EXPORT bool forward_Response(const remus::proto::Response& response,
                                                 zmq::socket_t* socket,
                                                 const zmq::SocketIdentity& client);

  //----------------------------------------------------------------------------
  //construct a response, the contents of the string will copied and sent.
  Response(remus::SERVICE_TYPE stype,
           const std::string& data,
           zmq::socket_t* socket,
           const zmq::SocketIdentity& client,
           SendMode mode);

  //----------------------------------------------------------------------------
  //create a response from reading from the socket
  explicit Response(zmq::socket_t* socket);

  bool send_impl(zmq::socket_t* socket,  const zmq::SocketIdentity& client, SendMode mode) const;

  remus::SERVICE_TYPE SType;
  bool Valid; //tells if the response is valid

  boost::shared_ptr<zmq::message_t> Storage;
};

}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif

#endif // remus_proto_Response_h
