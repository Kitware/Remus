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
  //the default remus server.
  ServerConnection():
    Endpoint(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",remus::BROKER_CLIENT_PORT).endpoint())
    {
    }

  //create a connection object that represent connection to a non
  //standard remus server
  explicit ServerConnection(std::string const& endpoint):
    Endpoint(endpoint)
    {
    assert(Endpoint.size() > 0);
    }

  //create a connection object that connects to the server specified by the
  //zmq::socketInfo. This is another way to connect to a non default server
  //with a custom protocol
  template<typename T>
  explicit ServerConnection(zmq::socketInfo<T> const& socket):
    Endpoint(socket.endpoint())
    {
    }

  //create a connection object that represent connection to a non
  //standard remus server
  ServerConnection(std::string const& hostName, int port):
    Endpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port).endpoint())

    {

    assert(hostName.size() > 0);
    assert(port > 0 && port < 65536);
    }

  inline std::string const& endpoint() const{ return Endpoint; }


private:
  std::string Endpoint;
};

}
}

#endif // __remus_serverConnection_h
