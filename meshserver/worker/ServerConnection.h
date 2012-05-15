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

#ifndef __meshserver_worker_serverConnection_h
#define __meshserver_worker_serverConnection_h

//included for symbol exports
#include "WorkerExports.h"

#include <meshserver/common/zmqHelper.h>
#include <meshserver/common/meshServerGlobals.h>

namespace meshserver{
namespace worker{

class MESHSERVERWORKER_EXPORT ServerConnection
{
public:
  //create a connection object that represents connecting to
  //the default meshserver server.
  ServerConnection():
    Endpoint(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",meshserver::BROKER_WORKER_PORT).endpoint())
    {
    }

  //create a connection object that represent connection to a non
  //standard meshserver server
  explicit ServerConnection(std::string const& endpoint):
    Endpoint(endpoint)
    {
    }

  //create a connection object that represent connection to a non
  //standard meshserver server
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

#endif // __meshserver_serverConnection_h
