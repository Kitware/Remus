/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_response_h
#define __meshserver_response_h

#include <cstddef>
#include <zmq.hpp>
#include "zmqHelper.h"
#include "meshServerGlobals.h"

#include <iostream>

namespace meshserver{
class JobResponse
{
public:
  JobResponse(const std::string& client);
  JobResponse(zmq::socket_t& socket);
  ~JobResponse();

  //Clears any existing data message, and reconstructs
  //the new message we will use when we call send
  template<typename T>
  void setData(const T& t);

  //Returns the data back to the user converted as the requested type
  //the presumption is that the client always knows the type of the response
  //it wants
  template<typename T>
  T dataAs();

  bool send(zmq::socket_t& socket) const;

private:
  void clearData();
  const std::string ClientAddress;
  zmq::message_t* Data;

  //make copying not possible
  JobResponse (const JobResponse&);
  void operator = (const JobResponse&);
};

//------------------------------------------------------------------------------
JobResponse::JobResponse(const std::string& client):
  ClientAddress(client),
  Data(NULL)
  {
  }

JobResponse::JobResponse(zmq::socket_t& socket):
  Data(NULL)
{
  zmq::stripSocketSig(socket);

  zmq::message_t data(0);
  socket.recv(&data);

  const std::size_t size = data.size();
  if(size>0)
    {
    this->Data = new zmq::message_t(size);
    this->Data->move(&data);
    }
}

//------------------------------------------------------------------------------
JobResponse::~JobResponse()
  {
  this->clearData();
  }

//------------------------------------------------------------------------------
void JobResponse::clearData()
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
T JobResponse::dataAs()
  {
  //default implementation works for primitive types
  return *reinterpret_cast<T*>(this->Data->data());
  }

//------------------------------------------------------------------------------
template<>
std::string JobResponse::dataAs<std::string>()
  {
  return std::string(static_cast<char*>(this->Data->data()),this->Data->size());
  }

//------------------------------------------------------------------------------
template<typename T>
void JobResponse::setData(const T& t)
  {
  this->clearData();

  const std::size_t size(sizeof(t));
  this->Data = new zmq::message_t(size);
  memcpy(this->Data->data(),&t,size);

  std::cout << "data content is: " << this->dataAs<T>() <<  "with size " << this->Data->size() << std::endl;
  }

//------------------------------------------------------------------------------
template<>
void JobResponse::setData<std::string>(const std::string& t)
  {
  this->clearData();
  this->Data = new zmq::message_t(t.size());
  memcpy(this->Data->data(),t.data(),t.size());
  std::cout << "data content is: " << this->dataAs<std::string>() << std::endl;
  }

//------------------------------------------------------------------------------
bool JobResponse::send(zmq::socket_t& socket) const
  {
  //if we have no data we will return false, since we couldn't send anything
  if(this->Data == NULL)
    {
    return false;
    }

  //we are sending our selves as a multi part message
  //frame 0: client address we need to route too
  //frame 1: fake rep spacer
  //frame 2: data
  zmq::message_t cAddress(this->ClientAddress.size());
  memcpy(cAddress.data(),this->ClientAddress.data(),this->ClientAddress.size());
  socket.send(cAddress,ZMQ_SNDMORE);

  zmq::message_t reqHeader(0);
  socket.send(reqHeader,ZMQ_SNDMORE);

  zmq::message_t realData;
  realData.move(this->Data);
  socket.send(realData);
  }
}

#endif // __meshserver_response_h
