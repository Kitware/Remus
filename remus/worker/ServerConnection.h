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

#ifndef __remus_worker_serverConnection_h
#define __remus_worker_serverConnection_h

#include <remus/common/zmqHelper.h>
#include <remus/common/remusGlobals.h>

namespace remus{
namespace worker{

class ServerConnection
{
public:
  //create a connection object that represents connecting to
  //the default remus server.
  ServerConnection():
    Endpoint(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",remus::BROKER_WORKER_PORT).endpoint())
    {
    }

  //create a connection object that represent connection to a non
  //standard remus server
  explicit ServerConnection(std::string const& endpoint):
    Endpoint(endpoint)
    {
    }

  //create a connection object that represent connection to a non
  //standard remus server
  explicit ServerConnection(std::string const& hostName, int port):
    Endpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port).endpoint())
    {
    }

  inline std::string const& endpoint() const{ return Endpoint; }

private:
  std::string Endpoint;
};

}
}

#endif // __remus_serverConnection_h
