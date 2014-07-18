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
#include <remus/proto/Response.h>

#include <remus/proto/zmqHelper.h>

namespace remus{
namespace proto{

//----------------------------------------------------------------------------
Response::Response(const zmq::SocketIdentity& client):
  ClientAddress(client),
  SType(remus::INVALID_SERVICE),
  Storage( new zmq::message_t() )
  {
  }

//----------------------------------------------------------------------------
Response::Response(zmq::socket_t* socket):
  ClientAddress(),
  SType(remus::INVALID_SERVICE),
  Storage( new zmq::message_t() )
{
  zmq::removeReqHeader(*socket);

  zmq::message_t servType;
  zmq::recv_harder(*socket,&servType);
  this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));

  zmq::recv_harder(*socket,this->Storage.get());
}

//------------------------------------------------------------------------------
void Response::setData(const std::string& t)
  {
  this->Storage->rebuild(t.size());
  memcpy(this->Storage->data(),t.data(),t.size());
  }

//------------------------------------------------------------------------------
const char* Response::data() const
{
  return this->Storage ? static_cast<char*>(this->Storage->data()) : NULL;
}

//------------------------------------------------------------------------------
std::size_t Response::dataSize() const
{
  return this->Storage ? this->Storage->size() : std::size_t(0);
}

//------------------------------------------------------------------------------
bool Response::send(zmq::socket_t *socket) const
{
  return this->send_impl(socket);
}

//------------------------------------------------------------------------------
bool Response::sendNonBlocking(zmq::socket_t *socket) const
{
  return this->send_impl(socket,ZMQ_DONTWAIT);
}

//------------------------------------------------------------------------------
bool Response::send_impl(zmq::socket_t* socket) const
  {
  //if we have no data we will return false, since we couldn't send anything
  if(!this->Storage)
    {
    return false;
    }

  //we are sending our selves as a multi part message
  //frame 0: client address we need to route too [optional]
  //frame 1: fake rep spacer
  //frame 2: Service Type we are responding too
  //frame 3: data
  if(this->ClientAddress.size()>0)
    {
    zmq::message_t cAddress(this->ClientAddress.size());
    memcpy(cAddress.data(),this->ClientAddress.data(),this->ClientAddress.size());
    zmq::send_harder(*socket,cAddress,ZMQ_SNDMORE);
    }

  zmq::attachReqHeader(*socket);

  zmq::message_t service(sizeof(this->SType));
  memcpy(service.data(),&this->SType,sizeof(this->SType));
  zmq::send_harder(*socket,service,ZMQ_SNDMORE);

  zmq::send_harder(*socket,*this->Storage.get());
  return true;
  }
}
}
