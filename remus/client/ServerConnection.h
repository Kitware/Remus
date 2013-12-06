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

#ifndef __remus_client_serverConnection_h
#define __remus_client_serverConnection_h

#include <remus/common/zmqHelper.h>
#include <remus/common/remusGlobals.h>
#include <assert.h>

namespace remus{
namespace client{

class ServerConnection
{
public:
  //create a connection object that represents connecting to
  //the default local host remus server.
  ServerConnection():
    Endpoint(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                  remus::SERVER_CLIENT_PORT).endpoint()),
    IsLocalEndpoint(true) //no need to call zmq::isLocalEndpoint
    {
    }

  //create a connection object that connects to the server specified by the
  //zmq::socketInfo. This is best way to connect to a non default server
  //with a custom protocol
  template<typename T>
  explicit ServerConnection(zmq::socketInfo<T> const& socket):
    Endpoint(socket.endpoint()),
    IsLocalEndpoint( zmq::isLocalEndpoint(socket) )
    {
    }

  //create a connection object that represent connection to a
  //standard tcp-ip remus server on a custom port
  ServerConnection(std::string const& hostName, int port):
    Endpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port).endpoint()),
    IsLocalEndpoint( zmq::isLocalEndpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port)) )
    {

    assert(hostName.size() > 0);
    assert(port > 0 && port < 65536);
    }

  inline std::string const& endpoint() const{ return Endpoint; }
  inline bool isLocalEndpoint() const{ return IsLocalEndpoint; }


private:
  std::string Endpoint;
  bool IsLocalEndpoint;
};

}
}

#endif // __remus_serverConnection_h
