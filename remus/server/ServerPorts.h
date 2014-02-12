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

#ifndef remus_server_ServerPorts_h
#define remus_server_ServerPorts_h

#include <string>
#include <remus/common/remusGlobals.h>
#include <remus/proto/zmq.hpp>
#include <remus/proto/zmqSocketInfo.h>

//included for symbol exports
#include <remus/server/ServerExports.h>

namespace remus{
namespace server{

//A class that holds the recommend ports for a remus server to bind too.
//This might not be the actual ports the server binds too, as they might
//be in use. This does allow the server though a starting point which
//if it can't bind tooo, it will sequentially try to bind to the next larger
//port number.

//This only supports tcp-ip port connections currently
//------------------------------------------------------------------------------
class REMUSSERVER_EXPORT ServerPorts
{

public:
  //default to loopback
  ServerPorts();

  //explicitly state the host name and port for both the client and worker
  ServerPorts(const std::string& clientHostName, unsigned int clientPort,
              const std::string& workerHostName, unsigned int workerPort);

  //will attempt to bind the passed in socket to client tcp-ip port we hold
  //if the bind fails, we will continue increasing the port number intill we find
  //a valid port. We will update our client socket info with the new valid information
  void bindClient(zmq::socket_t& socket);

  //will attempt to bind the passed in socket to worker tcp-ip port we hold
  //if the bind fails, we will continue increasing the port number intill we find
  //a valid port. We will update our worker socket info with the new valid information
  void bindWorker(zmq::socket_t& socket);

  const zmq::socketInfo<zmq::proto::tcp>& client() const
    { return this->Client; }
  const zmq::socketInfo<zmq::proto::tcp>& worker() const
    { return this->Worker; }

private:
  zmq::socketInfo<zmq::proto::tcp> Client;
  zmq::socketInfo<zmq::proto::tcp> Worker;
};

//end namespaces
}
}

#endif
