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

#ifndef remus_worker_serverConnection_h
#define remus_worker_serverConnection_h

#include <remus/common/remusGlobals.h>
#include <remus/proto/zmqSocketInfo.h>
#include <assert.h>

namespace remus{
namespace worker{

class ServerConnection
{
public:
  //create a connection object that represents connecting to
  //the default local host remus server.
  ServerConnection():
    Endpoint(zmq::socketInfo<zmq::proto::tcp>("127.0.0.1",
                                  remus::SERVER_WORKER_PORT).endpoint()),
    IsLocalEndpoint(true) //no need to call zmq::isLocalEndpoint
    {
    assert(Endpoint.size() > 0);
    }

  //create a connection object that connects to the server specified by the
  //zmq::socketInfo. This is best way to connect to a non default server
  //with a custom protocol
  template<typename T>
  explicit ServerConnection(zmq::socketInfo<T> const& socket):
    Endpoint(socket.endpoint()),
    IsLocalEndpoint(zmq::isLocalEndpoint(socket))
    {
    }

  //create a connection object that represent connection to a
  //standard tcp-ip remus server on a custom port
  explicit ServerConnection(std::string const& hostName, int port):
    Endpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port).endpoint()),
    IsLocalEndpoint( zmq::isLocalEndpoint(zmq::socketInfo<zmq::proto::tcp>(hostName,port)) )
    {
    assert(hostName.size() > 0);
    assert(port > 0 && port < 65536);
    }

  inline std::string const& endpoint() const{ return Endpoint; }
  inline bool isLocalEndpoint() const{ return IsLocalEndpoint; }

private:
  std::string Endpoint;
  bool IsLocalEndpoint;
};


//convert a string in the form of proto://hostname:port where :port
//is optional into a server connection class
//if we fail to parse the string we will return an instance of
//the default server connection
inline
remus::worker::ServerConnection make_ServerConnection(const std::string& dest)
{
  remus::worker::ServerConnection connection;

  //tcp:// and ipc:// + 1 character are the minimum size
  if(dest.size() > 6)
    {
    const std::string::size_type protocall_end_pos = dest.find("://");
    if(protocall_end_pos != std::string::npos)
      {
      //determine if we are tcp
      //we can protocalll_end_pos as a distance currently since we are starting
      //at zero
      const std::string protocall = dest.substr(0,protocall_end_pos-1);
      if(protocall == zmq::proto::scheme_name(zmq::proto::tcp()))
        {
        //we need to extract the host name and port
        const std::string::size_type host_end_pos = dest.rfind(":");
        if(host_end_pos!=protocall_end_pos)
          {
          //extract the host_name and port
          std::string host_name = dest.substr(protocall_end_pos+3,
                                              host_end_pos);

          const std::string::size_type port_len = dest.size() - host_end_pos;
          const std::string temp_port(dest, host_end_pos+1, port_len );
          int port = boost::lexical_cast<int>(temp_port);

          zmq::socketInfo<zmq::proto::tcp> socket(host_name,port);
          connection = remus::worker::ServerConnection(socket);
          }
        }
      else if(protocall == zmq::proto::scheme_name(zmq::proto::ipc()))
        {
        //3 equals size of "://"
        const std::string host_name = dest.substr(protocall_end_pos+3);
        zmq::socketInfo<zmq::proto::ipc> socket(host_name);
        connection = remus::worker::ServerConnection(socket);
        }
      else if(protocall == zmq::proto::scheme_name(zmq::proto::inproc()))
        {
        //3 equals size of "://"
        const std::string host_name = dest.substr(protocall_end_pos+3);
        zmq::socketInfo<zmq::proto::inproc> socket(host_name);
        connection = remus::worker::ServerConnection(socket);
        }
      }
    }
  return connection;
}

}
}

#endif // remus_serverConnection_h
