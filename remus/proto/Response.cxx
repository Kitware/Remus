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

#include <boost/make_shared.hpp>

namespace remus{
namespace proto{

//----------------------------------------------------------------------------
Response::Response(remus::SERVICE_TYPE stype, const std::string& rdata):
  SType(stype),
  FullyFormed(true),
  Storage( boost::make_shared<zmq::message_t>(rdata.size()) )
{
  memcpy(this->Storage->data(),rdata.data(),rdata.size());
}

//----------------------------------------------------------------------------
Response::Response(remus::SERVICE_TYPE stype,
                   const char* rdata, std::size_t size):
  SType(stype),
  FullyFormed(true),
  Storage( boost::make_shared<zmq::message_t>(size) )
{
  memcpy(this->Storage->data(),rdata,size);
}

//----------------------------------------------------------------------------
Response::Response(zmq::socket_t* socket):
  SType(remus::INVALID_SERVICE),
  FullyFormed(false), //false by default in case we failed to recv everything
  Storage( boost::make_shared<zmq::message_t>() )
{
  const bool removedHeader = zmq::removeReqHeader(*socket);

  if(removedHeader)
    {
    zmq::message_t servType;
    const bool parsedServiceType = zmq::recv_harder(*socket,&servType);
    if(parsedServiceType)
      {
      this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));
      const bool recvStorage = zmq::recv_harder(*socket,this->Storage.get());
      //if recvStorage is true than we received every chunk of data and we
      //are valid
      this->FullyFormed = recvStorage;
      }
    }
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
bool Response::send(zmq::socket_t *socket,
                    const zmq::SocketIdentity& client) const
{
  return this->send_impl(socket,client);
}

//------------------------------------------------------------------------------
bool Response::sendNonBlocking(zmq::socket_t *socket,
                               const zmq::SocketIdentity& client) const
{
  return this->send_impl(socket,client,ZMQ_DONTWAIT);
}

//------------------------------------------------------------------------------
bool Response::send_impl(zmq::socket_t* socket,
                         const zmq::SocketIdentity& client,
                         int flags) const
{
  //we are sending our selves as a multi part message
  //frame 0: client address we need to route too
  //frame 1: fake rep spacer
  //frame 2: Service Type we are responding too
  //frame 3: data

  bool responseSent = false;
  zmq::message_t cAddress(client.size());
  memcpy(cAddress.data(),client.data(),client.size());
  const bool sentClientAddress = zmq::send_harder( *socket,
                                               cAddress, flags|ZMQ_SNDMORE );
  if(sentClientAddress)
    {
    const bool sentFakeReq = zmq::attachReqHeader(*socket,flags);
    if(sentFakeReq)
      {
      zmq::message_t service(sizeof(this->SType));
      memcpy(service.data(),&this->SType,sizeof(this->SType));

      const bool sentServiceType = zmq::send_harder( *socket,
                                                 service, flags|ZMQ_SNDMORE );
      if(sentServiceType)
        {
        responseSent = zmq::send_harder( *socket, *this->Storage.get(), flags);
        }
      }
    }
  return responseSent;
}

}
}
