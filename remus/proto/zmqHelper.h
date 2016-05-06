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

//We now provide our own zmq.hpp since it has been removed from zmq 3, and
//made its own project
#include <remus/proto/zmq.hpp>
#include <remus/proto/zmqSocketIdentity.h>
#include <remus/proto/zmqSocketInfo.h>

//included for export symbols
#include <remus/proto/ProtoExports.h>

//inject some basic zero MQ helper functions into the zmq namespace that
//are publicly available to consumers of the RemusProto library
namespace zmq
{
REMUSPROTO_EXPORT
bool address_send(zmq::socket_t & socket, const zmq::SocketIdentity& address);

REMUSPROTO_EXPORT
zmq::SocketIdentity address_recv(zmq::socket_t& socket);

//------------------------------------------------------------------------------
REMUSPROTO_EXPORT
void connectToAddress(zmq::socket_t &socket,const std::string &endpoint);

template<typename T>
inline
void connectToAddress(zmq::socket_t &socket,const zmq::socketInfo<T> &sInfo)
{
  const std::string endpoint = sInfo.endpoint();
  connectToAddress(socket,endpoint);
}

//------------------------------------------------------------------------------
//A wrapper around zeroMQ poll. When we call the standard poll call
//we can experience high number of system level interrupts which
//cause zero to throw an exception.
//So we keep track of how long we desire to poll for, and keep going
//through interrupts until we hit out timeout
//Note: poll_safely doesn't support infinite polling
REMUSPROTO_EXPORT
void poll_safely(zmq_pollitem_t *items, int nitems, boost::int64_t timeout);

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//bind to initialInfo socket, and return that socket info
template<typename T>
inline
zmq::socketInfo<T> bindToAddress(zmq::socket_t &socket,
                                 const zmq::socketInfo<T>& initialInfo)
{
  set_socket_linger(socket);
  socket.bind(initialInfo.endpoint().c_str());
  return initialInfo;
}

//------------------------------------------------------------------------------
//return the true port we have bound to, since the initial one might be already in use
template< >
inline
zmq::socketInfo<zmq::proto::tcp> bindToAddress(zmq::socket_t &socket,
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

} //namespace zmq

//collection of methods that are private and can only be used by classes
//that are within the RemusProto library
namespace zmq
{
//------------------------------------------------------------------------------
//A wrapper around zeroMQ send. When we call the standard send call
//from a Qt class we experience high number of system level interrupts which
//cause zero to throw an exception when we are sending a blocking message.
//When sending a blocking message we will try a couple of times before
//giving up
//In the future we need to change the client server
//communication in Remus to be async instead of req/reply based.
bool send_harder(zmq::socket_t& socket, zmq::message_t& message, int flags=0);

//------------------------------------------------------------------------------
//A wrapper around zeroMQ recv. When we call the standard recv call
//from a Qt class we experience high number of system level interrupts which
//cause zero to throw an exception when we are recv a blocking message.
//When recving a blocking message we will try a couple of times before
//giving up
//In the future we need to change the client server
//communication in Remus to be async instead of req/reply based.
bool recv_harder(zmq::socket_t& socket, zmq::message_t* message, int flags=0);

//------------------------------------------------------------------------------
//we presume that every message needs to be stripped
//as we make everything act like a req/rep and pad
//a null message on everything
//Returns true if we removed the ReqHeader, or if no header
//needs to be removed
bool removeReqHeader(zmq::socket_t& socket, int flags=0);

//------------------------------------------------------------------------------
//we presume that every message needs to be treated like a Req/Rep
//message and we need to pad a null message on everything
//Returns true if we added the ReqHeader, or if no header
//needs to be added
bool attachReqHeader(zmq::socket_t& socket,int flags=0);

} //namespace zmq


#endif // remus_proto_zmqHelper_h
