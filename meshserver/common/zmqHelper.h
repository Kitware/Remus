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
#include <boost/lexical_cast.hpp>

#include <meshserver/common/zmqTraits.h>

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{
//holds the identity of a zero mq socket in a way that is
//easier to copy around
struct socketIdentity
{
  socketIdentity(char* data, std::size_t size):
    Size(size)
    {
    memcpy(Data,data,size);
    }

  socketIdentity():
    Size(0)
    {}

  bool operator ==(const socketIdentity& b) const
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
//for the socket info class we are going to separate out the port of the tcp

//templated based on the transport types


template<typename Transport>
struct socketInfo
{

  socketInfo():Host(){}
  explicit socketInfo(const std::string& hostName):Host(hostName){}
  std::string endpoint() const {return  zmq::to_string(Transport()) + Host;}
  const std::string& host() const{ return Host; }

private:
  std::string Host;
};

template<> struct socketInfo<zmq::proto::tcp>
{
  socketInfo():Host(),Port(-1){}
  explicit socketInfo(const std::string& hostName, int port):Host(hostName),Port(port){}
  std::string endpoint() const
    {
    return  zmq::to_string(zmq::proto::tcp()) + Host + ":" + boost::lexical_cast<std::string>(Port);
    }

  const std::string& host() const{ return Host; }
  int port() const { return Port; }

  void setPort(int p){ Port=p; }

private:
  int Port;
  std::string Host;
};

inline std::string to_string(const zmq::socketIdentity& add)
{
  return std::string(add.data(),add.size());
}

inline void connectToAddress(zmq::socket_t &socket,const std::string &endpoint)
{
  socket.connect(endpoint.c_str());
}

template<typename T>
inline void connectToAddress(zmq::socket_t &socket,const zmq::socketInfo<T> &sInfo)
{
  socket.connect(sInfo.endpoint().c_str());
}

inline zmq::socketInfo<zmq::proto::tcp> bindToTCPSocket(zmq::socket_t &socket,
                                                        int port)
{
  //given a default port try to connect using tcp on that port, continue
  //till we hit maximum number of ports and if still failing throw execption
  zmq::socketInfo<zmq::proto::tcp> socketInfo("*",port);
  //go through all ports, I hope the input port is inside the Ephemeral range
  int rc = -1;
  int i=port;
  for(;i < 65535 && rc != 0; ++i)
    {
    socketInfo.setPort(i);
    //using the C syntax to skip having to catch the exception;
    rc = zmq_bind(socket.operator void *(),socketInfo.endpoint().c_str());
    }

  if(rc!=0)
    {
    std::cout << "tcp socket binding failed" << std::endl;
    throw zmq::error_t();
    }
  return socketInfo;
}

template<typename Proto>
inline zmq::socketInfo<Proto> bindToAddress(zmq::socket_t &socket, const std::string &address)
{
  //given a default port try to connect using tcp on that port, continue
  //till we hit maximum number of ports and if still failing throw execption
  zmq::socketInfo<Proto> socketInfo(address);
  socket.bind(socketInfo.endpoint().c_str());
  return socketInfo;
}

static bool address_send(zmq::socket_t & socket, const zmq::socketIdentity& address)
{
  zmq::message_t message(address.size());
  memcpy(message.data(), address.data(), address.size());
  return socket.send(message);
}

static zmq::socketIdentity address_recv(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return zmq::socketIdentity((char*)message.data(),message.size());
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
