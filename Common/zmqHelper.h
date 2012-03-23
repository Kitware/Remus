/*=========================================================================
  
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.
  
=========================================================================*/


#ifndef __zeroHelper_h
#define __zeroHelper_h

#include <zmq.hpp>
#include <sstream>

//A collection of general helper methods

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{
inline void connectToSocket(zmq::socket_t &socket, const int num)
{
  std::stringstream buffer;
  buffer << "tcp://127.0.0.1:" << num;
  socket.connect(buffer.str().c_str());
}

inline void bindToSocket(zmq::socket_t &socket, const int num)
{
  std::stringstream buffer;
  buffer << "tcp://127.0.0.1:" << num;
  socket.bind(buffer.str().c_str());
}

bool s_send(zmq::socket_t & socket, const std::string& contents)
{
  zmq::message_t message(contents.size());
  memcpy(message.data(), contents.data(), contents.size());
  return socket.send(message);
}

std::string s_recv(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return std::string(static_cast<char*>(message.data()),message.size());
}

//if a socket is type REP or REQ it prepends each message
//with a null frame. So this common function removes that null frame
void stripSocketSig(zmq::socket_t& socket)
{
  int socketType;
  std::size_t socketTypeSize = sizeof(socketType);
  socket.getsockopt(ZMQ_TYPE,&socketType,&socketTypeSize);
  if(socketType == ZMQ_ROUTER || socketType == ZMQ_DEALER)
    {
    zmq::message_t reqHeader;
    socket.recv(&reqHeader);
    }
}
}

#endif // __zeroHelper_h
