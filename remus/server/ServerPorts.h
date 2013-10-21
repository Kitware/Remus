
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

#ifndef __remus_server_ServerPorts_h
#define __remus_server_ServerPorts_h

#include <string>
#include <remus/common/zmqHelper.h>
#include <remus/common/remusGlobals.h>

namespace remus{
namespace server{

//A class that holds the recommend ports for a remus server to bind too.
//This might not be the actual ports the server binds too, as they might
//be in use. This does allow the server though a starting point which
//if it can't bind tooo, it will sequentially try to bind to the next larger
//port number.

//This only supports tcp-ip port connections currently
//------------------------------------------------------------------------------
class ServerPorts
{

public:
  //default to loopback
  ServerPorts():
    Client("127.0.0.1",remus::SERVER_CLIENT_PORT),
    Worker("127.0.0.1",remus::SERVER_WORKER_PORT)
  {

  }

  ServerPorts(std::string const& clientHostName, unsigned int clientPort,
              std::string const& workerHostName, unsigned int workerPort):
    Client(clientHostName,clientPort),
    Worker(workerHostName,workerPort)
  {
    assert(clientHostName.size() > 0);
    assert(clientPort > 0 && clientPort < 65536);

    assert(workerHostName.size() > 0);
    assert(workerPort > 0 && workerPort < 65536);

    assert(workerHostName == clientHostName && workerPort == clientPort);
  }

  //will attempt to bind the passed in socket to client tcp-ip port we hold
  //if the bind fails, we will continue increasing the port number intill we find
  //a valid port. We will update our client socket info with the new valid information
  void bindClient(zmq::socket_t& socket)
  {
    this->Client = zmq::bindToTCPSocket(socket,this->Client);
  }

  //will attempt to bind the passed in socket to worker tcp-ip port we hold
  //if the bind fails, we will continue increasing the port number intill we find
  //a valid port. We will update our worker socket info with the new valid information
  void bindWorker(zmq::socket_t& socket)
  {
    this->Worker = zmq::bindToTCPSocket(socket,this->Worker);
  }

  const zmq::socketInfo<zmq::proto::tcp>& client() const { return this->Client; }
  const zmq::socketInfo<zmq::proto::tcp>& worker() const { return this->Worker; }



protected:
  zmq::socketInfo<zmq::proto::tcp> Client;
  zmq::socketInfo<zmq::proto::tcp> Worker;
};

//end namespaces
}
}

#endif