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
#include <remus/testing/Testing.h>

#include <string>

namespace {

std::string make_endPoint(std::string host, int port)
{
  const std::string conn_type( zmq::to_string(zmq::proto::tcp()) );
  std::string temp( conn_type + host + ":" +
                                  boost::lexical_cast<std::string>(port) );
  return temp;
}


bool verify_ports(remus::server::ServerPorts ports,
             std::string client_host, int client_port,
             std::string worker_host, int worker_port)
{
  //verify that pre binding to a server port
  //that we have the correct details

  //verify that the details are correct
  bool valid = true;
  REMUS_VALID( (ports.client().host() == client_host), valid);
  REMUS_VALID( (ports.client().port() == client_port), valid );
  REMUS_VALID( (ports.worker().host() == worker_host), valid );
  REMUS_VALID( (ports.worker().port() == worker_port), valid );

  //now verify that the strings match what we expect
  REMUS_VALID( (make_endPoint(client_host,client_port) ==
                  ports.client().endpoint()), valid );
  REMUS_VALID( (make_endPoint(worker_host,worker_port) ==
                             ports.worker().endpoint()), valid );

  return valid;

}

bool verify_bindings(remus::server::ServerPorts ports)
{
  bool valid = true;

  //bind the sockets manually and verify that the binding works.
  zmq::context_t context(1);

  zmq::socket_t client_socket(context,ZMQ_ROUTER);
  zmq::socket_t worker_socket(context,ZMQ_ROUTER);

  ports.bindClient(client_socket);
  ports.bindWorker(worker_socket);


  //verify that we are bound by trying to manually bind again
  //to the port using raw zmq
  zmq::socket_t check_socket(context,ZMQ_ROUTER);
  zmq::socketInfo<zmq::proto::tcp> check_socket_info;

  //check client socket is bound
  check_socket_info = ports.client();
  int rc = zmq_bind(check_socket.operator void *(),
                    check_socket_info.endpoint().c_str());

  //verify that we can't bind to the socket since the client is already bound
  //to it
  REMUS_VALID( (rc != 0), valid);


  check_socket_info = ports.worker();
  rc = zmq_bind(check_socket.operator void *(),
                    check_socket_info.endpoint().c_str());

  //verify that we can't bind to the socket since the worker is already bound
  //to it
  REMUS_VALID( (rc != 0), valid);


  return valid;
}

} //namespace


int UnitTestServerPorts(int, char *[])
{
  //verify the defaults
  REMUS_ASSERT( verify_ports(
                remus::server::ServerPorts(),
                "127.0.0.1",remus::SERVER_CLIENT_PORT,
                "127.0.0.1",remus::SERVER_WORKER_PORT)
                );

  //verify the explicit constructor
  //don't try to bind to this server
  remus::server::ServerPorts bad_server( "66.38.239.255", 0,
                                         "90.78.56.34", 1);
  REMUS_ASSERT( verify_ports(
                bad_server,
                "66.38.239.255", 0,
                "90.78.56.34", 1)
                );

  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts()) );

  //we reach here we have a valid server ports test
  return 0;
}
