#ifndef __worker_h
#define __worker_h

#include "zmq.hpp"
#include <sstream>
#include <iostream>

namespace meshserver
{
class Worker
{
public:

Worker(const int socket_num):
  SocketNum(socket_num),
  Context(1),
  MeshJobs(this->Context, ZMQ_PULL),
  MeshStatus(this->Context, ZMQ_PUSH)
  {
  this->connectToSocket(this->MeshJobs,this->SocketNum);
  this->connectToSocket(this->MeshStatus,this->SocketNum+10);
  }

bool execute()
{
  std::cout << "Executing" <<std::endl;
  while(true)
    {
    zmq::message_t message;
    this->MeshJobs.recv(&message);

    std::istringstream iss(static_cast<char*>(message.data()));
    std::cout << "worker sent message: " << iss.str() << std::endl;
    sleep(1);
    //  Send results to sink
    message.rebuild();
    this->MeshStatus.send(message);
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

  const int SocketNum;
  zmq::context_t Context;
  zmq::socket_t MeshJobs;
  zmq::socket_t MeshStatus;
};
}


#endif
