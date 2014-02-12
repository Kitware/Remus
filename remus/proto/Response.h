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

#ifndef remus_proto_Response_h
#define remus_proto_Response_h

#include <remus/common/MeshIOType.h>
#include <remus/proto/zmqHelper.h>

namespace remus{
namespace proto{
class Response
{
public:
  //----------------------------------------------------------------------------
  explicit Response(const zmq::socketIdentity& client):
    ClientAddress(client),
    SType(remus::INVALID_SERVICE),
    Data(NULL)
    {
    }

  //----------------------------------------------------------------------------
  explicit Response(zmq::socket_t& socket):
    ClientAddress(),
    SType(remus::INVALID_SERVICE),
    Data(NULL)
  {
    zmq::removeReqHeader(socket);

    zmq::message_t servType;
    zmq::recv_harder(socket,&servType);
    this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));

    zmq::message_t data(0);
    zmq::recv_harder(socket,&data);
    const std::size_t size = data.size();
    if(size>0)
      {
      this->Data = new zmq::message_t(size);
      this->Data->move(&data);
      }
  }
  //----------------------------------------------------------------------------
  ~Response() { this->clearData(); }

  //Clears any existing data message, and reconstructs
  //the new message we will use when we call send
  template<typename T>
  void setData(const T& t);

  //Returns the data back to the user converted as the requested type
  //the presumption is that the client always knows the type of the response
  //it wants
  template<typename T>
  T dataAs();

  //Set the service type that this response is responding too.
  //By default the service type is set to invalid if you don't specify one.
  void setServiceType(remus::SERVICE_TYPE type) { SType = type; }
  remus::SERVICE_TYPE serviceType() const { return SType; }

  bool send(zmq::socket_t& socket) const;

private:
  void clearData();
  const zmq::socketIdentity ClientAddress;
  remus::SERVICE_TYPE SType;
  zmq::message_t* Data;

  //make copying not possible
  Response (const Response&);
  void operator = (const Response&);
};

//------------------------------------------------------------------------------
inline void Response::clearData()
  {
  //if the response has been sent than the DataMessage is valid to be
  //deleted because we have moved the contents into a different message
  //which zeroMQ is now managing
  if(Data)
    {
    delete Data;
    }
  }

//------------------------------------------------------------------------------
template<typename T>
inline T Response::dataAs()
  {
  //default implementation works for primitive types
  return *reinterpret_cast<T*>(this->Data->data());
  }

//------------------------------------------------------------------------------
template<>
inline std::string Response::dataAs<std::string>()
  {
  return std::string(static_cast<char*>(this->Data->data()),this->Data->size());
  }

//------------------------------------------------------------------------------
template<typename T>
inline void Response::setData(const T& t)
  {
  this->clearData();

  const std::size_t size(sizeof(t));
  this->Data = new zmq::message_t(size);
  memcpy(this->Data->data(),&t,size);
  //std::cout << "data content is: " << this->dataAs<T>() << std::endl;
  }

//------------------------------------------------------------------------------
template<>
inline void Response::setData<std::string>(const std::string& t)
  {
  this->clearData();
  this->Data = new zmq::message_t(t.size());
  memcpy(this->Data->data(),t.data(),t.size());
  //std::cout << "data content is: " << this->dataAs<std::string>() << std::endl;
  }

//------------------------------------------------------------------------------
inline bool Response::send(zmq::socket_t& socket) const
  {
  //if we have no data we will return false, since we couldn't send anything
  if(this->Data == NULL)
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
    zmq::send_harder(socket,cAddress,ZMQ_SNDMORE);
    }

  zmq::attachReqHeader(socket);

  zmq::message_t service(sizeof(this->SType));
  memcpy(service.data(),&this->SType,sizeof(this->SType));
  zmq::send_harder(socket,service,ZMQ_SNDMORE);

  zmq::message_t realData;
  realData.move(this->Data);
  zmq::send_harder(socket,realData);
  return true;
  }
}
}

#endif // remus_proto_Response_h
