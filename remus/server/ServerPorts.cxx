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
#include <remus/server/PortNumbers.h>
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
  Context( remus::server::make_Context() ),
  Client(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                          remus::server::CLIENT_PORT)),
  Worker(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                          remus::server::WORKER_PORT)),
  Status(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                          remus::server::STATUS_PORT))
{
  assert(remus::server::CLIENT_PORT > 0 && remus::server::CLIENT_PORT < 65536);
  assert(remus::server::WORKER_PORT > 0 && remus::server::WORKER_PORT < 65536);
  assert(remus::server::STATUS_PORT > 0 && remus::server::STATUS_PORT < 65536);

  assert(remus::server::CLIENT_PORT != remus::server::WORKER_PORT);
  assert(remus::server::STATUS_PORT != remus::server::CLIENT_PORT);
  assert(remus::server::STATUS_PORT != remus::server::WORKER_PORT);
}

//------------------------------------------------------------------------------
ServerPorts::ServerPorts(const std::string& clientHostName,
                         unsigned int clientPort,
                         const std::string& workerHostName,
                         unsigned int workerPort):
  Context( remus::server::make_Context() ),
  Client(zmq::socketInfo<zmq::proto::tcp>(clientHostName,clientPort)),
  Worker(zmq::socketInfo<zmq::proto::tcp>(workerHostName,workerPort)),
  Status(zmq::socketInfo<zmq::proto::tcp>(clientHostName,
                                          remus::server::STATUS_PORT))
{
  assert(clientHostName.size() > 0);
  assert(clientPort > 0 && clientPort < 65536);

  assert(workerHostName.size() > 0);
  assert(workerPort > 0 && workerPort < 65536);

  assert(!(workerHostName == clientHostName && workerPort == clientPort));
}

//------------------------------------------------------------------------------
ServerPorts::ServerPorts(const std::string& clientHostName,
                         unsigned int clientPort,
                         unsigned int statusPort,
                         const std::string& workerHostName,
                         unsigned int workerPort):
  Context( remus::server::make_Context() ),
  Client(zmq::socketInfo<zmq::proto::tcp>(clientHostName,clientPort)),
  Worker(zmq::socketInfo<zmq::proto::tcp>(workerHostName,workerPort)),
  Status(zmq::socketInfo<zmq::proto::tcp>(clientHostName,statusPort))
{
  assert(clientHostName.size() > 0);
  assert(clientPort > 0 && clientPort < 65536);

  assert(workerHostName.size() > 0);
  assert(workerPort > 0 && workerPort < 65536);

  assert(statusPort > 0 && statusPort < 65536);

  //verify that when worker and client host name are the same we dont
  //have any ports that are the same value
  assert(!(workerHostName == clientHostName && workerPort == clientPort));
  assert(!(workerHostName == clientHostName && workerPort == statusPort));

  //the status and client port can never be the same as they always share
  //the same host name
  assert(statusPort != clientPort);
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

//------------------------------------------------------------------------------
void ServerPorts::bindStatus(zmq::socket_t* socket)
{
  this->Status = detail::bind(*socket,this->Status);
}

//------------------------------------------------------------------------------
boost::shared_ptr<zmq::context_t> make_Context(std::size_t threads)
{
  return boost::make_shared<zmq::context_t>( static_cast<int>(threads) );
}


//end namespaces
}
}
