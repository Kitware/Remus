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

#ifndef __meshserver_common_zeroHelper_h
#define __meshserver_common_zeroHelper_h

#include <zmq.hpp>
#include <sstream>

//A collection of general helper methods

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{

struct socketAddress
{
  socketAddress(char* data, std::size_t size):
    Size(size)
    {
    memcpy(Data,data,size);
    }

  socketAddress():
    Size(0)
    {}

  bool operator ==(const socketAddress& b) const
  {
    if(this->size() != b.size()) { return false; }
    return 0 == (memcmp(this->data(),b.data(),this->size()));
  }

  const char* data() const { return &Data[0]; }
  std::size_t size() const { return Size; }

private:
  std::size_t Size;
  char Data[256];
};

//holds the information needed to construct a zmq socket connection
//from zmq documentation:
//endpoint argument is a string consisting of two parts as follows:
//transport ://address. The transport part specifies the underlying transport
//protocol to use. The meaning of the address part is specific to the underlying
//transport protocol selected.

struct socketInfo
{
  socketInfo(): Transport(),Address(){}

  socketInfo(const std::string& transport, const std::string& address):
    Transport(transport), Address(address) {}

  const std::string endpoint() const
    {
    return Transport + "://" + Address;
    }


  const std::string Transport;
  const std::string Address;
};


inline std::string to_string(const zmq::socketAddress& add)
{
  return std::string(add.data(),add.size());
}

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

static bool address_send(zmq::socket_t & socket, const zmq::socketAddress& address)
{
  zmq::message_t message(address.size());
  memcpy(message.data(), address.data(), address.size());
  return socket.send(message);
}

static zmq::socketAddress address_recv(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return zmq::socketAddress((char*)message.data(),message.size());
}

static void empty_send(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.send(message);
  return;
}

static void empty_recv(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return;
}

//we presume that every message needs to be stripped
//as we make everything act like a req/rep and pad
//a null message on everything
static void removeReqHeader(zmq::socket_t& socket)
{
  int socketType;
  std::size_t socketTypeSize = sizeof(socketType);
  socket.getsockopt(ZMQ_TYPE,&socketType,&socketTypeSize);
  if(socketType != ZMQ_REQ && socketType != ZMQ_REP)
    {
    zmq::message_t reqHeader;
    socket.recv(&reqHeader);
    }
}

//if we are not a req or rep socket make us look like one
 static void attachReqHeader(zmq::socket_t& socket)
{
  int socketType;
  std::size_t socketTypeSize = sizeof(socketType);
  socket.getsockopt(ZMQ_TYPE,&socketType,&socketTypeSize);
  if(socketType != ZMQ_REQ && socketType != ZMQ_REP)
    {
    zmq::message_t reqHeader(0);
    socket.send(reqHeader,ZMQ_SNDMORE);
    }
}

}

#endif // __meshserver_common_zeroHelper_h
