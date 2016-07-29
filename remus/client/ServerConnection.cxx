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

#include <remus/client/ServerConnection.h>

#include <assert.h>
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <remus/server/PortNumbers.h>
#include <remus/proto/zmq.hpp>

namespace remus{
namespace client{

//------------------------------------------------------------------------------
ServerConnection::ServerConnection():
  Context( remus::client::make_ServerContext() ),
  Endpoint(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                          remus::server::CLIENT_PORT).endpoint()),
  IsLocalEndpoint(true) //no need to call zmq::isLocalEndpoint
{
}

//------------------------------------------------------------------------------
ServerConnection::ServerConnection(const std::string& hostName, int port):
  Context( remus::client::make_ServerContext() ),
  Endpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port).endpoint()),
  IsLocalEndpoint( zmq::isLocalEndpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port)) )
{
  assert(hostName.size() > 0);
  assert(port > 0 && port < 65536);
}

//------------------------------------------------------------------------------
remus::client::ServerConnection make_ServerConnection(const std::string& dest)
{
  //convert a string in the form of proto://hostname:port where :port
  //is optional into a server connection class
  //if we fail to parse the string we will return an instance of
  //the default server connection
  remus::client::ServerConnection connection;

  //tcp:// and ipc:// + 1 character are the minimum size
  if(dest.size() > 6)
    {
    const std::string::size_type protocol_end_pos = dest.find("://");
    if(protocol_end_pos != std::string::npos)
      {
      //determine if we are tcp
      //we can protocol_end_pos as a distance currently since we are starting
      //at zero
      const std::string protocol = dest.substr(0,protocol_end_pos);
      if(protocol == zmq::proto::scheme_name(zmq::proto::tcp()))
        {
        //we need to extract the host name and port
        const std::string::size_type host_end_pos = dest.rfind(":");
        if(host_end_pos!=protocol_end_pos)
          {
          //extract the host_name and port
          const std::string::size_type host_len = host_end_pos -
                                                       (protocol_end_pos+3);
          const std::string::size_type port_len = dest.size() - host_end_pos;

          const std::string host_name(dest, protocol_end_pos+3, host_len);
          const std::string temp_port(dest, host_end_pos+1, port_len );
          int port = boost::lexical_cast<int>(temp_port);


          zmq::socketInfo<zmq::proto::tcp> socket(host_name,port);
          connection = remus::client::ServerConnection(socket);
          }
        }
      else if(protocol == zmq::proto::scheme_name(zmq::proto::ipc()))
        {
        //3 equals size of "://"
        const std::string host_name = dest.substr(protocol_end_pos+3);
        zmq::socketInfo<zmq::proto::ipc> socket(host_name);
        connection = remus::client::ServerConnection(socket);
        }
      else if(protocol == zmq::proto::scheme_name(zmq::proto::inproc()))
        {
        //3 equals size of "://"
        const std::string host_name = dest.substr(protocol_end_pos+3);
        zmq::socketInfo<zmq::proto::inproc> socket(host_name);
        connection = remus::client::ServerConnection(socket);
        }
      }
    }
  return connection;
}

//------------------------------------------------------------------------------
boost::shared_ptr<zmq::context_t> make_ServerContext(std::size_t threads)
{
  return boost::make_shared<zmq::context_t>( static_cast<int>(threads) );
}


}
}
