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
#include <remus/testing/Testing.h>

#include <string>
#include <cstring>

namespace {

std::string make_endPoint(std::string host, int port)
{
  const std::string conn_type( zmq::proto::scheme_and_separator(zmq::proto::tcp()) );
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

  zmq::socket_t client_socket(context,ZMQ_REP);
  zmq::socket_t worker_socket(context,ZMQ_REP);

  ports.bindClient(&client_socket);
  ports.bindWorker(&worker_socket);


  //verify that we are bound by trying to manually bind again
  //to the port using raw zmq
  zmq::socket_t check_client(context,ZMQ_REQ);
  zmq::socket_t check_worker(context,ZMQ_REQ);

  //get the information of the socket to connect to
  remus::server::PortConnection client_socket_info = ports.client();
  remus::server::PortConnection worker_socket_info = ports.worker();

  int rc = zmq_connect(check_client.operator void *(),
                       client_socket_info.endpoint().c_str());
  REMUS_VALID( (rc == 0), valid);

  rc = zmq_connect(check_worker.operator void *(),
                   worker_socket_info.endpoint().c_str());
  REMUS_VALID( (rc == 0), valid);

  //send data on the client and worker
  int clientTag = 1, workerTag = 2;
  {
  zmq::message_t client_data(sizeof(clientTag));
  std::memcpy(client_data.data(),&clientTag,sizeof(clientTag));
  zmq::send_harder(check_client,client_data);

  zmq::message_t worker_data(sizeof(workerTag));
  std::memcpy(worker_data.data(),&workerTag,sizeof(workerTag));
  zmq::send_harder(check_worker,worker_data);
  }

  {
  //grab the data from the client and worker sockets we just bound
  zmq::message_t client_data_recv;
  zmq::recv_harder(client_socket,&client_data_recv);
  int client_recv_tag = *(reinterpret_cast<int*>(client_data_recv.data()));
  REMUS_VALID( (clientTag == client_recv_tag), valid);

  zmq::message_t worker_data_recv;
  zmq::recv_harder(worker_socket,&worker_data_recv);
  int worker_recv_tag = *(reinterpret_cast<int*>(worker_data_recv.data()));
  REMUS_VALID( (workerTag == worker_recv_tag), valid);
  }

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
  remus::server::ServerPorts bad_server( "66.38.239.255", 1,
                                         "90.78.56.34", 2);
  REMUS_ASSERT( verify_ports(
                bad_server,
                "66.38.239.255", 1,
                "90.78.56.34", 2)
                );

  //verify everything works with dual tcp-ip
  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts()) );

  //verify everything works with dual inproc
  zmq::socketInfo<zmq::proto::inproc> ci("client_channel");
  zmq::socketInfo<zmq::proto::inproc> wi("worker_channel");
  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts(ci,wi)) );

  //now mix ipc and tcp-ip
  zmq::socketInfo<zmq::proto::tcp> default_ctcp("127.0.0.1",
                                               remus::SERVER_CLIENT_PORT);

  //now mix inproc and tcp-ip
  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts(default_ctcp,wi)) );

  #ifndef _WIN32
  //verify everything works with dual ipc
  zmq::socketInfo<zmq::proto::ipc> c("client_channel");
  zmq::socketInfo<zmq::proto::ipc> w("worker_channel");
  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts(c,w)) );

  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts(default_ctcp,w)) );

  //now mix inproc and ipc
  REMUS_ASSERT( verify_bindings(remus::server::ServerPorts(c,wi)) );
  #endif

  //we reach here we have a valid server ports test
  return 0;
}
