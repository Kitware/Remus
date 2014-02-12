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

namespace remus{
namespace server{

//------------------------------------------------------------------------------
ServerPorts::ServerPorts():
  Client("127.0.0.1",remus::SERVER_CLIENT_PORT),
  Worker("127.0.0.1",remus::SERVER_WORKER_PORT)
{
}

//------------------------------------------------------------------------------
ServerPorts::ServerPorts(const std::string& clientHostName,
                         unsigned int clientPort,
                         const std::string& workerHostName,
                         unsigned int workerPort):
  Client(clientHostName,clientPort),
  Worker(workerHostName,workerPort)
{
  assert(clientHostName.size() > 0);
  assert(clientPort > 0 && clientPort < 65536);

  assert(workerHostName.size() > 0);
  assert(workerPort > 0 && workerPort < 65536);

  assert(workerHostName != clientHostName && workerPort != clientPort);
}

//------------------------------------------------------------------------------
void ServerPorts::bindClient(zmq::socket_t& socket)
{
  this->Client = zmq::bindToTCPSocket(socket,this->Client);
}

//------------------------------------------------------------------------------
void ServerPorts::bindWorker(zmq::socket_t& socket)
{
  this->Worker = zmq::bindToTCPSocket(socket,this->Worker);
}

//end namespaces
}
}
