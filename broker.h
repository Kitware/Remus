#ifndef __server_h
#define __server_h

#include <iostream>
#include <sstream>

#include "zmq.hpp"

namespace meshserver
{
class Broker
{
public:
  Broker(const int client_socket, const int worker_socket):
  ClientSocketNum(client_socket),
  WorkerSocketNum(worker_socket),
  Context(1),
  MeshRequests(this->Context,ZMQ_PULL),
  Workers(this->Context,ZMQ_PUSH),
  WorkerStatus(this->Context, ZMQ_PULL),
  MeshStatus(this->Context, ZMQ_PUB)
  {
  this->bindToSocket(MeshRequests,ClientSocketNum);
  this->bindToSocket(Workers,WorkerSocketNum);

  this->bindToSocket(WorkerStatus,WorkerSocketNum+10);
  this->bindToSocket(MeshStatus,ClientSocketNum+10);

  zmq::device(ZMQ_STREAMER,this->MeshRequests,this->Workers);
  }

bool execute()
{
  //loop forever waiting for worker status, and pushing it
  while(true)
    {
    this->s_recv(this->WorkerStatus);
    this->s_send(this->MeshStatus,"Mesh Complete");
    }
  return true;
}

private:

void connectToSocket(zmq::socket_t &socket, const int num)
{
  std::stringstream buffer;
  buffer << "tcp://127.0.0.1:" << num;
  socket.connect(buffer.str().c_str());
}

void bindToSocket(zmq::socket_t &socket, const int num)
{
  std::stringstream buffer;
  buffer << "tcp://127.0.0.1:" << num;
  socket.bind(buffer.str().c_str());
}

bool s_send (zmq::socket_t & socket, const std::string & string)
{
  zmq::message_t message(string.size());
  memcpy(message.data(), string.data(), string.size());

  bool rc = socket.send(message);
  return (rc);
}

std::string s_recv (zmq::socket_t & socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return std::string(static_cast<char*>(message.data()), message.size());
}

  const int ClientSocketNum;
  const int WorkerSocketNum;

  zmq::context_t Context;

  zmq::socket_t MeshRequests;
  zmq::socket_t Workers;

  zmq::socket_t MeshStatus;
  zmq::socket_t WorkerStatus;
};

}


#endif
