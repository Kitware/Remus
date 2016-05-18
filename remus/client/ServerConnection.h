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

#ifndef remus_client_ServerConnection_h
#define remus_client_ServerConnection_h

#include <remus/common/CompilerInformation.h>
//included for export symbols
#include <remus/client/ClientExports.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/shared_ptr.hpp>
#include <remus/proto/zmqSocketInfo.h>
REMUS_THIRDPARTY_POST_INCLUDE

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif


//forward declare the context
namespace zmq { class context_t; }

namespace remus{
namespace client{

class REMUSCLIENT_EXPORT ServerConnection
{
public:
  //create a connection object that represents connecting to
  //the default local host remus server.
  ServerConnection();

  //create a connection object that connects to the server specified by the
  //zmq::socketInfo. This is best way to connect to a non default server
  //with a custom protocol
  template<typename T>
  explicit ServerConnection(zmq::socketInfo<T> const& socket);

  //create a connection object that represent connection to a
  //standard tcp-ip remus server on a custom port
  ServerConnection(std::string const& hostName, int port);

  inline std::string const& endpoint() const{ return Endpoint; }
  inline bool isLocalEndpoint() const{ return IsLocalEndpoint; }

  //we have to leak some details to support inproc communication
  boost::shared_ptr<zmq::context_t> context() const { return this->Context; }

  //don't overwrite the context of a server connection once the server
  //connection is passed to the worker, as that will cause undefined behavior
  //and most likely will crash the program
  void context(boost::shared_ptr<zmq::context_t> c) { this->Context = c; }

private:
  boost::shared_ptr<zmq::context_t> Context;
  std::string Endpoint;
  bool IsLocalEndpoint;
};

//convert a string in the form of proto://hostname:port where :port
//is optional into a server connection class
//if we fail to parse the string we will return an instance of
//the default server connection
REMUSCLIENT_EXPORT
remus::client::ServerConnection make_ServerConnection(const std::string& dest);

//construct a server context that can be shared between many server connections
REMUSCLIENT_EXPORT
boost::shared_ptr<zmq::context_t> make_ServerContext(std::size_t num_threads=1);

//------------------------------------------------------------------------------
template<typename T>
ServerConnection::ServerConnection(zmq::socketInfo<T> const& socket):
  Context( remus::client::make_ServerContext() ),
  Endpoint(socket.endpoint()),
  IsLocalEndpoint( zmq::isLocalEndpoint(socket) )
{
}

}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif

#endif // remus_ServerConnection_h
