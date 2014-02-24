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

#ifndef remus_proto_Message_h
#define remus_proto_Message_h

#include <boost/shared_ptr.hpp>

#include <remus/common/ConditionalStorage.h>
#include <remus/common/MeshIOType.h>
#include <remus/proto/zmqHelper.h>

namespace remus{
namespace proto{
class Message
{
public:
  //----------------------------------------------------------------------------
  //pass in a data string the job message will copy and send
  Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype, const std::string& data):
    MType(mtype),
    SType(stype),
    Data(NULL),
    Size(0),
    ValidMsg(true),
    Storage(data)
    {
    this->Data = this->Storage.get();
    this->Size = this->Storage.size();
    }

  //----------------------------------------------------------------------------
  //pass in a data pointer that the message will use when sending
  //the pointer data can't become invalid before you call send.
  Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype, const char* data, int size):
    MType(mtype),
    SType(stype),
    Data(data),
    Size(size),
    ValidMsg(true),
    Storage()
    {
    }

  //----------------------------------------------------------------------------
  //creates a job message with no data
  Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype):
    MType(mtype),
    SType(stype),
    Data(NULL),
    Size(0),
    ValidMsg(true),
    Storage()
    {
    }

  //----------------------------------------------------------------------------
  //creates a job message from reading in the socket
  Message(zmq::socket_t &socket)
    {
    //we are receiving a multi part message
    //frame 0: REQ header / attachReqHeader does this
    //frame 1: Mesh Type
    //frame 2: Service Type
    //frame 3: Job Data //optional
    zmq::more_t more;
    size_t more_size = sizeof(more);
    socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);

    //construct a job message from the socket
    zmq::removeReqHeader(socket);

    zmq::message_t MeshIOType;
    zmq::recv_harder(socket,&MeshIOType);
    this->MType = *(reinterpret_cast<remus::common::MeshIOType*>(MeshIOType.data()));

    zmq::message_t servType;
    zmq::recv_harder(socket,&servType);
    this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));

    socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
    if(more>0)
      {
      zmq::message_t data;
      zmq::recv_harder(socket,&data);
      remus::common::ConditionalStorage tempStorage(data);
      this->Storage.swap( tempStorage );
      this->Size = this->Storage.size();
      this->Data = this->Storage.get();
      }
    else
      {
      this->Size = 0;
      this->Data = NULL;
      }

    //see if we have more data. If so we need to say we are invalid
    //as we parsed the wrong type of message
    socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
    ValidMsg=(more==0)?true:false;
    }

  //send a blocking message
  bool send(zmq::socket_t& socket) const;

  //send the message as non blocking so that we can do
  //async sending
  bool sendNonBlocking(zmq::socket_t& socket) const;

  void releaseData() { this->Data = NULL; this->Size = 0;}

  const remus::common::MeshIOType& MeshIOType() const { return MType; }
  const remus::SERVICE_TYPE& serviceType() const { return SType; }

  const char* data() const { return Data; }
  int dataSize() const { return Size; }

  bool isValid() const { return ValidMsg; }

  template<typename T>
  void dump(T& t) const;

private:
  bool send_impl(zmq::socket_t& socket, int flags = 0) const;

  remus::common::MeshIOType MType;
  remus::SERVICE_TYPE SType;
  const char* Data; //points to the message contents to send
  int Size;
  bool ValidMsg; //tells if the message is valid, mainly used by the server

  //Storage is an optional allocation that is used when we need to have
  //ownership over that data being received or sent.
  remus::common::ConditionalStorage Storage;
};


//------------------------------------------------------------------------------
inline bool Message::send(zmq::socket_t &socket) const
{
  return this->send_impl(socket);
}

inline bool Message::sendNonBlocking(zmq::socket_t &socket) const
{
  return this->send_impl(socket,ZMQ_DONTWAIT);
}

inline bool Message::send_impl(zmq::socket_t &socket, int flags) const
{
  //we are sending our selves as a multi part message
  //frame 0: REQ header / attachReqHeader does this
  //frame 1: Mesh Type
  //frame 2: Service Type
  //frame 3: Job Data //optional

  if(!this->ValidMsg)
    {
    return false;
    }
  zmq::attachReqHeader(socket);

  bool valid = true;
  zmq::message_t MeshIOType(sizeof(this->MType));
  memcpy(MeshIOType.data(),&this->MType,sizeof(this->MType));
  valid = valid && zmq::send_harder(socket,MeshIOType,flags|ZMQ_SNDMORE);

  zmq::message_t service(sizeof(this->SType));
  memcpy(service.data(),&this->SType,sizeof(this->SType));
  if(this->Size> 0 && valid)
    {
    //send the service line not as the last line
    valid = zmq::send_harder(socket,service,flags|ZMQ_SNDMORE);

    zmq::message_t data(this->Size);
    memcpy(data.data(),this->Data,this->Size);

    valid = valid && zmq::send_harder(socket,data,flags);
    }
  else if(valid) //we are done
    {
    valid = zmq::send_harder(socket,service,flags);
    }
  return valid;
}

//------------------------------------------------------------------------------
template<typename T>
void Message::dump(T& t) const
  {
  //dump the info to the t stream
  t << "Valid: " << this->isValid() << std::endl;
  t << "Mesh Type: " << this->MeshIOType() << std::endl;
  t << "Serivce Type: " << this->serviceType() << std::endl;
  t << "Size: " << this->dataSize() << std::endl;
  if(this->dataSize() > 0)
    {
    t << "Data: " << std::string(this->Data,this->dataSize()) << std::endl;
    }
  }
}
}
#endif //remus_Message_h
