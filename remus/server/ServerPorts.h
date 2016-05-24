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

#include <remus/common/CompilerInformation.h>
#include <remus/server/ServerExports.h>


REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
#include <remus/proto/zmqSocketInfo.h>
REMUS_THIRDPARTY_POST_INCLUDE

#include <string>

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif


namespace zmq { class context_t; class socket_t; }

namespace remus{
namespace server{

//A lightweight helper class that hides the
class REMUSSERVER_EXPORT PortConnection
{
public:
  template<typename T>
  PortConnection(const zmq::socketInfo<T>& socket):
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
//if it can't bind too, it will sequentially try to bind to the next larger
//port number.
//
//Remus is designed so that the client needs to access both the client
//connection and the status connection. So for this to happen properly the
//ServerPorts will force the status port to use the clientHostName when
//using tcp-ip connection. When using the zmq::socketInfo constructor
//we require the client and status port to shared the same type
//
//
//------------------------------------------------------------------------------
class REMUSSERVER_EXPORT ServerPorts
{

public:
  //default to loopback tcp connection for client, worker, and status
  ServerPorts();

  //explicitly state the host name and port for both the client and worker
  //this will explicitly create tcp connection for booth client and worker
  //this will cause the status port to use the clientHostName and the
  //default status port.
  //Note: The status port uses the clientHostName so that
  //clients can access the status feed
  ServerPorts(const std::string& clientHostName, unsigned int clientPort,
              const std::string& workerHostName, unsigned int workerPort);

  //explicitly state the host name and port for the client, status and worker
  //this will explicitly create tcp connections for all ports, and
  //will make the status port bind to the clientHostName
  //Note: The status port uses the clientHostName so that
  //clients can access the status feed
  ServerPorts(const std::string& clientHostName,
              unsigned int clientPort,
              unsigned int statusPort, //uses client host name
              const std::string& workerHostName, unsigned int workerPort);

  //explicitly state the connection type, this can handle inproc, ipc,
  //and tcp connection types
  template<typename ClientType, typename WorkerType>
  ServerPorts(const zmq::socketInfo<ClientType>&  c,
              const zmq::socketInfo<ClientType>&  s,
              const zmq::socketInfo<WorkerType>&  w);

  //will attempt to bind the passed in socket to client port connection endpoint
  //that we where constructed with. If that is a tcp-ip endpoing and the bind
  //fails we will continue increasing the port number intill we find
  //a valid port. We will update our client socket info with the new valid information
  //Requires: socket to be non NULL
  void bindClient(zmq::socket_t* socket);

  //will attempt to bind the passed in socket to worker port connection endpoint
  //that we where constructed with. If that is a tcp-ip endpoing and the bind
  //fails we will continue increasing the port number intill we find
  //a valid port. We will update our worker socket info with the new valid information
  //Requires: socket to be non NULL
  void bindWorker(zmq::socket_t* socket);

  //will attempt to bind the passed in socket to status port connection endpoint
  //that we where constructed with. If that is a tcp-ip endpoing and the bind
  //fails we will continue increasing the port number intill we find
  //a valid port. We will update our status socket info with the new valid information
  //Requires: socket to be non NULL
  void bindStatus(zmq::socket_t* socket);

  const PortConnection& client() const
    { return this->Client; }
  const PortConnection& worker() const
    { return this->Worker; }
  const PortConnection& status() const
    { return this->Status; }

  //we have to leak some details to support inproc communication
  boost::shared_ptr<zmq::context_t> context() const { return this->Context; }

  //don't overwrite the context of a server once you start brokering.
  //As that will cause undefined behavior and most likely will crash the program
  void context(boost::shared_ptr<zmq::context_t> c) { this->Context = c; }

private:
  boost::shared_ptr<zmq::context_t> Context;
  PortConnection Client;
  PortConnection Worker;
  PortConnection Status;
};

//construct a context that is used for both the client and worker comms
REMUSSERVER_EXPORT
boost::shared_ptr<zmq::context_t> make_Context(std::size_t num_threads=1);


//------------------------------------------------------------------------------
template<typename ClientType, typename WorkerType>
ServerPorts::ServerPorts(zmq::socketInfo<ClientType> const& c,
                         zmq::socketInfo<ClientType> const& s,
                         zmq::socketInfo<WorkerType> const& w):
  Context( remus::server::make_Context() ),
  Client(c),
  Worker(w),
  Status(s)
{
}

//end namespaces
}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif

#endif
