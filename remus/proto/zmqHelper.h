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

#ifndef remus_proto_zmqHelper_h
#define remus_proto_zmqHelper_h

#include <remus/common/CompilerInformation.h>
#include <remus/common/Timer.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/lexical_cast.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

//We now provide our own zmq.hpp since it has been removed from zmq 3, and
//made its own project
#include <remus/proto/zmq.hpp>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/proto/zmqSocketInfo.h>

#include <algorithm>
#include <cstddef>
#include <sstream>

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{
inline bool address_send(zmq::socket_t & socket, const zmq::SocketIdentity& address)
{
  zmq::message_t message(address.size());

  std::copy(address.data(),
            address.data()+address.size(),
            static_cast<char*>(message.data()));

  return socket.send(message);
}

inline zmq::SocketIdentity address_recv(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return zmq::SocketIdentity((char*)message.data(),message.size());
}

//specify a default linger so that if what we are connecting to
//doesn't exist and we are told to shutdown we don't hang for ever
inline void set_socket_linger(zmq::socket_t &socket)
{
  const int linger_duration = 250;
  socket.setsockopt(ZMQ_LINGER, &linger_duration, sizeof(int) );

  //ToDo we need to determine which sockets we want to set timeouts
  //on for sending and receiving messages.
  //I am worried about how this is going to interact with the variable
  //query / poll rates that happen on OSX 10.9+ when an app starts to be
  //slowed down
// #if ZMQ_VERSION_MAJOR >= 3
//   socket.setsockopt(ZMQ_RCVTIMEO, &linger_duration, sizeof(int) );
//   socket.setsockopt(ZMQ_SNDTIMEO, &linger_duration, sizeof(int) );
// #endif
}

//bind to initialInfo socket, and return that socket info
template<typename T>
inline zmq::socketInfo<T> bindToAddress(zmq::socket_t &socket,
                                        const zmq::socketInfo<T>& initialInfo)
{
  set_socket_linger(socket);
  socket.bind(initialInfo.endpoint().c_str());
  return initialInfo;
}

//return the true port we have bound to, since the initial one might be already in use
template< >
inline zmq::socketInfo<zmq::proto::tcp> bindToAddress(zmq::socket_t &socket,
                        const zmq::socketInfo<zmq::proto::tcp>& initialInfo)
{
  set_socket_linger(socket);

  //go through all ports, I hope the input port is inside the Ephemeral range
  int rc = -1;
  zmq::socketInfo<zmq::proto::tcp> socketInfo(initialInfo);
  for(int i=socketInfo.port();i < 65535 && rc != 0; ++i)
    {
    socketInfo.setPort(i);
    //using the C syntax to skip having to catch the exception;
    const std::string endpoint = socketInfo.endpoint();
    rc = zmq_bind(socket.operator void *(),endpoint.c_str());
    }

  if(rc!=0)
    {
    throw zmq::error_t();
    }

    return socketInfo;
}


inline void connectToAddress(zmq::socket_t &socket,const std::string &endpoint)
{
  set_socket_linger(socket);
  int rc = zmq_connect(socket.operator void*(), endpoint.c_str());
  //check if we bound properly, if so stop
  if(rc == 0)
    {
    return;
    }
  //if we failed to bind properly lets try a couple more times if it was because
  //the socket was already in-use. This fixes issues with connecting/disconnecting
  //the same socket repeatedly, something tests do.
  int error_value = zmq_errno();
  for(int i=0; i < 5 && rc != 0 && error_value == EADDRINUSE ; ++i)
    {
    rc = zmq_connect(socket.operator void*(), endpoint.c_str());
    }
}

template<typename T>
inline void connectToAddress(zmq::socket_t &socket,const zmq::socketInfo<T> &sInfo)
{
  const std::string endpoint = sInfo.endpoint();
  connectToAddress(socket,endpoint);
}

//A wrapper around zeroMQ poll. When we call the standard poll call
//we can experience high number of system level interrupts which
//cause zero to throw an exception.
//So we keep track of how long we desire to poll for, and keep going
//through interrupts until we hit out timeout
//Note: poll_safely doesn't support infinite polling
inline void poll_safely(zmq_pollitem_t *items,
                        int nitems,
                        boost::int64_t timeout)
{
  assert(timeout > 0);

  remus::common::Timer timer;
  int rc = -1;

  while(timeout >= 0 && rc < 0)
    {
    rc = zmq_poll(items, nitems, static_cast<long>(ZMQ_POLL_MSEC*timeout) );
    if(rc < 0 && zmq_errno() == EINTR)
      {
      //figure out how long we polled for, subtract that from our timeout
      //and continue to poll
      boost::int64_t delta = timer.elapsed();
      timeout -= delta;
      }
    else if( rc < 0)
      {
      //not an EINTR, so cascade this up to the calling code
      throw zmq::error_t();
      }

    //reset the time each time so that the delta we compute is only
    //for this single iteration
    timer.reset();
    }
}

//A wrapper around zeroMQ send. When we call the standard send call
//from a Qt class we experience high number of system level interrupts which
//cause zero to throw an exception when we are sending a blocking message.
//When sending a blocking message we will try a couple of times before
//giving up
//In the future we need to change the client server
//communication in Remus to be async instead of req/reply based.
inline bool send_harder(zmq::socket_t& socket, zmq::message_t& message, int flags=0)
{
  bool sent = false;
  short tries = 0;
  while(!sent && tries < 5)
    {
    try
      {
      sent = socket.send(message,flags);
      if (!sent) { ++tries; }
      }
    catch(zmq::error_t){ ++tries; }
    }
  return sent;
}

//A wrapper around zeroMQ recv. When we call the standard recv call
//from a Qt class we experience high number of system level interrupts which
//cause zero to throw an exception when we are recv a blocking message.
//When recving a blocking message we will try a couple of times before
//giving up
//In the future we need to change the client server
//communication in Remus to be async instead of req/reply based.
inline bool recv_harder(zmq::socket_t& socket, zmq::message_t* message, int flags=0)
{
  bool recieved = false;
  short tries = 0;
  while(!recieved && tries < 5)
    {
    try
      {
      recieved = socket.recv(message,flags);
      if (!recieved) { ++tries; }
      }
    catch(zmq::error_t){ ++tries; }
    }
  return recieved;
}

//we presume that every message needs to be stripped
//as we make everything act like a req/rep and pad
//a null message on everything
//Returns true if we removed the ReqHeader, or if no header
//needs to be removed
inline bool removeReqHeader(zmq::socket_t& socket,
                            int flags=0)
{
  bool removedHeader = true;
  int socketType;
  std::size_t socketTypeSize = sizeof(socketType);
  socket.getsockopt(ZMQ_TYPE,&socketType,&socketTypeSize);
  if(socketType != ZMQ_REQ && socketType != ZMQ_REP)
    {
    zmq::message_t reqHeader;
    try
      {
      removedHeader = zmq::recv_harder(socket, &reqHeader, flags);
      }
    catch(zmq::error_t)
      {
      removedHeader = false;
      }
    }
  return removedHeader;
}

//we presume that every message needs to be treated like a Req/Rep
//message and we need to pad a null message on everything
//Returns true if we added the ReqHeader, or if no header
//needs to be added
 inline bool attachReqHeader(zmq::socket_t& socket,
                             int flags=0)
{
  bool attachedHeader = true;
  int socketType;
  std::size_t socketTypeSize = sizeof(socketType);
  socket.getsockopt(ZMQ_TYPE,&socketType,&socketTypeSize);
  if(socketType != ZMQ_REQ && socketType != ZMQ_REP)
    {
    zmq::message_t reqHeader(0);
    try
      {
      attachedHeader = zmq::send_harder(socket, reqHeader, flags|ZMQ_SNDMORE);
      }
    catch(zmq::error_t err)
      {
      attachedHeader = false;
      }
    }
  return attachedHeader;
}

}

#endif // remus_proto_zmqHelper_h
