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

#include <boost/scoped_ptr.hpp>

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
  explicit Response(const zmq::SocketIdentity& client);

  //----------------------------------------------------------------------------
  explicit Response(zmq::socket_t* socket);

  //----------------------------------------------------------------------------
  ~Response();

  void setData(const std::string& t);
  std::string data() const;

  //Set the service type that this response is responding too.
  //By default the service type is set to invalid if you don't specify one.
  void setServiceType(remus::SERVICE_TYPE type) { SType = type; }
  remus::SERVICE_TYPE serviceType() const { return SType; }

  bool send(zmq::socket_t* socket) const;

private:
  const zmq::SocketIdentity ClientAddress;
  remus::SERVICE_TYPE SType;
  boost::scoped_ptr<zmq::message_t> Storage;

  //make copying not possible
  Response (const Response&);
  void operator = (const Response&);
};

}
}

#endif // remus_proto_Response_h
