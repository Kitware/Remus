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

//MSVC has a warning for using "unsafe" algorithms such as copy that can
//easily overrun the end of unchecked pointers. The problem is that if any
//header includes xutility before we set this pragma nothing will happen
#include <remus/common/CompilerInformation.h>
#ifdef REMUS_MSVC
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include <remus/common/Timer.h>
#include <remus/proto/zmqHelper.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/lexical_cast.hpp>
REMUS_THIRDPARTY_POST_INCLUDE


#include <algorithm>
#include <cstddef>
#include <sstream>

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{

//------------------------------------------------------------------------------
bool address_send(zmq::socket_t & socket, const zmq::SocketIdentity& address)
{
  zmq::message_t message(address.size());

  std::copy(address.data(),
            address.data()+address.size(),
            static_cast<char*>(message.data()));

  return socket.send(message);
}

//------------------------------------------------------------------------------
zmq::SocketIdentity address_recv(zmq::socket_t& socket)
{
  zmq::message_t message;
  socket.recv(&message);
  return zmq::SocketIdentity((char*)message.data(),message.size());
}

//------------------------------------------------------------------------------
void connectToAddress(zmq::socket_t &socket,const std::string &endpoint)
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

//------------------------------------------------------------------------------
//A wrapper around zeroMQ poll. When we call the standard poll call
//we can experience high number of system level interrupts which
//cause zero to throw an exception.
//So we keep track of how long we desire to poll for, and keep going
//through interrupts until we hit out timeout
//Note: poll_safely doesn't support infinite polling
void poll_safely(zmq_pollitem_t *items, int nitems, boost::int64_t timeout)
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
bool send_harder(zmq::socket_t& socket, zmq::message_t& message, int flags)
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

//------------------------------------------------------------------------------
//A wrapper around zeroMQ recv. When we call the standard recv call
//from a Qt class we experience high number of system level interrupts which
//cause zero to throw an exception when we are recv a blocking message.
//When recving a blocking message we will try a couple of times before
//giving up
//In the future we need to change the client server
//communication in Remus to be async instead of req/reply based.
bool recv_harder(zmq::socket_t& socket, zmq::message_t* message, int flags)
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

//------------------------------------------------------------------------------
//we presume that every message needs to be stripped
//as we make everything act like a req/rep and pad
//a null message on everything
//Returns true if we removed the ReqHeader, or if no header
//needs to be removed
bool removeReqHeader(zmq::socket_t& socket, int flags)
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

//------------------------------------------------------------------------------
//we presume that every message needs to be treated like a Req/Rep
//message and we need to pad a null message on everything
//Returns true if we added the ReqHeader, or if no header
//needs to be added
bool attachReqHeader(zmq::socket_t& socket, int flags)
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

#ifdef REMUS_MSVC
#pragma warning(pop)
#endif
