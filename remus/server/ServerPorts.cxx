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

#include <remus/server/ServerPorts.h>
#include <remus/proto/zmqHelper.h>

#include <boost/make_shared.hpp>


//the one class that doesn't use zmq::bindToAddress as it is the only class
//that has a use case where it is binding to TCP-IP and need to
namespace detail
{

remus::server::PortConnection bind(zmq::socket_t &socket,
                                   const remus::server::PortConnection& conn)
{
  if(conn.scheme() == zmq::proto::scheme_name(zmq::proto::tcp()))
    {
    zmq::socketInfo<zmq::proto::tcp> sinfo(conn.host(),conn.port());
    return remus::server::PortConnection( zmq::bindToAddress(socket,sinfo) );
    }
  else if(conn.scheme() == zmq::proto::scheme_name(zmq::proto::ipc()))
    {
    zmq::socketInfo<zmq::proto::ipc> sinfo(conn.host());
    return remus::server::PortConnection( zmq::bindToAddress(socket,sinfo) );
    }
  else if(conn.scheme() == zmq::proto::scheme_name(zmq::proto::inproc()))
    {
    zmq::socketInfo<zmq::proto::inproc> sinfo(conn.host());
    return remus::server::PortConnection( zmq::bindToAddress(socket,sinfo) );
    }
  else
    { //exceptional case we can't bind to anything.
    throw zmq::error_t();
    }
}


}


namespace remus{
namespace server{

//------------------------------------------------------------------------------
ServerPorts::ServerPorts():
  Context( boost::make_shared<zmq::context_t>(2) ),
  Client(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                          remus::SERVER_CLIENT_PORT)),
  Worker(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                          remus::SERVER_WORKER_PORT))
{
  assert(remus::SERVER_CLIENT_PORT > 0 && remus::SERVER_CLIENT_PORT < 65536);
  assert(remus::SERVER_WORKER_PORT > 0 && remus::SERVER_WORKER_PORT < 65536);
  assert(remus::SERVER_CLIENT_PORT != remus::SERVER_WORKER_PORT);
}

//------------------------------------------------------------------------------
ServerPorts::ServerPorts(const std::string& clientHostName,
                         unsigned int clientPort,
                         const std::string& workerHostName,
                         unsigned int workerPort):
  Context( boost::make_shared<zmq::context_t>(2) ),
  Client(zmq::socketInfo<zmq::proto::tcp>(clientHostName,clientPort)),
  Worker(zmq::socketInfo<zmq::proto::tcp>(workerHostName,workerPort))
{
  assert(clientHostName.size() > 0);
  assert(clientPort > 0 && clientPort < 65536);

  assert(workerHostName.size() > 0);
  assert(workerPort > 0 && workerPort < 65536);

  assert(workerHostName != clientHostName && workerPort != clientPort);
}

//------------------------------------------------------------------------------
void ServerPorts::bindClient(zmq::socket_t* socket)
{
  this->Client = detail::bind(*socket,this->Client);
}

//------------------------------------------------------------------------------
void ServerPorts::bindWorker(zmq::socket_t* socket)
{
  this->Worker = detail::bind(*socket,this->Worker);
}

//end namespaces
}
}
