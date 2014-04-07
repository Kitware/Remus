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

#include <boost/shared_ptr.hpp>

//included for export symbols
#include <remus/server/ServerExports.h>

namespace zmq { class context_t; class socket_t; }

namespace remus{
namespace server{

//A lightweight helper class that hides the
class REMUSSERVER_EXPORT PortConnection
{
public:
  template<typename SocketType>
  PortConnection(const SocketType& socket):
    Endpoint(socket.endpoint()),
    Host(socket.host()),
    Scheme(socket.scheme()),
    Port(socket.port())
  {}

  //returns a valid zmq endpoint string to use for a connection or bind
  const std::string& endpoint() const { return this->Endpoint; }

  //returns the host name section of the endpoint
  const std::string& host() const { return this->Host; }

  //returns the zmq transport scheme can be tcp, ipc or inproc
  const std::string& scheme() const { return this->Scheme; }

  //returns the port number for tcp connections, for other scheme types
  //will return -1
  int port() const { return this->Port; }

private:
  std::string Endpoint;
  std::string Host;
  std::string Scheme;
  int Port;
};

//A class that holds the recommend ports for a remus server to bind too.
//This might not be the actual ports the server binds too, as they might
//be in use. This does allow the server though a starting point which
//if it can't bind tooo, it will sequentially try to bind to the next larger
//port number.

//------------------------------------------------------------------------------
class REMUSSERVER_EXPORT ServerPorts
{

public:
  //default to loopback tcp connection for client and worker
  ServerPorts();

  //explicitly state the host name and port for both the client and worker
  //this will explicitly create tcp connection for booth client and worker
  ServerPorts(const std::string& clientHostName, unsigned int clientPort,
              const std::string& workerHostName, unsigned int workerPort);

  //explicitly state the connection type, this can handle inproc, ipc,
  //and tcp connection types
  template<typename ClientSocketType, typename WokerSocketType>
  ServerPorts(const ClientSocketType& c,
              const WokerSocketType& w):
    Client(c),
    Worker(w)
  { }

  //will attempt to bind the passed in socket to client tcp-ip port we hold
  //if the bind fails, we will continue increasing the port number intill we find
  //a valid port. We will update our client socket info with the new valid information
  void bindClient(zmq::socket_t* socket);

  //will attempt to bind the passed in socket to worker tcp-ip port we hold
  //if the bind fails, we will continue increasing the port number intill we find
  //a valid port. We will update our worker socket info with the new valid information
  void bindWorker(zmq::socket_t* socket);

  const PortConnection& client() const
    { return this->Client; }
  const PortConnection& worker() const
    { return this->Worker; }

  //we have to leak some details to support inproc communication
  boost::shared_ptr<zmq::context_t> context() const { return this->Context; }

  //don't overwrite the context of a server once you start brokering.
  //As that will cause undefined behavior and most likely will crash the program
  void context(boost::shared_ptr<zmq::context_t> c) { this->Context = c; }

private:
  boost::shared_ptr<zmq::context_t> Context;
  PortConnection Client;
  PortConnection Worker;
};

//end namespaces
}
}

#endif
