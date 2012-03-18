#ifndef __worker_h
#define __worker_h

#include "zmq.hpp"
#include <sstream>
#include <iostream>

namespace meshserver
{
class Client
{
public:

Client(const int socket_num):
  SocketNum(socket_num),
  Context(1),
  Server(this->Context, ZMQ_REQ)
  {
  this->connectToSocket(this->Server,this->SocketNum);
  }

bool execute()
{
  for(int i=0; i < 10; i++)
    {
    this->s_send(this->Server, "I want a mesh job!");
    std::cout << "Client recieved msg: " << this->s_recv(this->Server) << std::endl;
    }

  this->Server.close();
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

  const int SocketNum;
  zmq::context_t Context;
  zmq::socket_t Server;
};
}


#endif
