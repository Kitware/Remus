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

#include <remus/proto/Message.h>

#include <remus/proto/zmq.hpp>
#include <remus/proto/zmqHelper.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/make_shared.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <cstring>

namespace remus{
namespace proto{

//----------------------------------------------------------------------------
Message send_NonBlockingMessage(remus::common::MeshIOType mtype,
                                remus::SERVICE_TYPE stype,
                                const std::string& data,
                                zmq::socket_t* socket)
{
  return Message(mtype,stype,data,socket,Message::NonBlocking);
}

//----------------------------------------------------------------------------
Message send_NonBlockingMessage(remus::common::MeshIOType mtype,
                                remus::SERVICE_TYPE stype,
                                zmq::socket_t* socket)
{
  return Message(mtype,stype,socket,Message::NonBlocking);
}


//----------------------------------------------------------------------------
Message send_Message(remus::common::MeshIOType mtype,
                     remus::SERVICE_TYPE stype,
                     const std::string& data,
                     zmq::socket_t* socket)
{
  return Message(mtype,stype,data,socket,Message::Blocking);
}

//----------------------------------------------------------------------------
Message send_Message(remus::common::MeshIOType mtype,
                     remus::SERVICE_TYPE stype,
                     zmq::socket_t* socket)
{
  return Message(mtype,stype,socket,Message::Blocking);
}

//----------------------------------------------------------------------------
//parse a message from a socket
Message receive_Message( zmq::socket_t* socket )
{
  return Message(socket);
}

//----------------------------------------------------------------------------
//forward a message that has been received to another socket
bool forward_Message(const remus::proto::Message& message,
                     zmq::socket_t* socket)
{
  return message.send_impl(socket,Message::Blocking);
}


//----------------------------------------------------------------------------
Message::Message(remus::common::MeshIOType mtype,
                 remus::SERVICE_TYPE stype,
                 const std::string& mdata,
                 zmq::socket_t* socket,
                 Message::SendMode mode):
  MType(mtype),
  SType(stype),
  Valid(true), //need to be initially valid to be sent
  Storage( boost::make_shared<zmq::message_t>(mdata.size()) )
{
  std::memcpy(Storage->data(),mdata.data(),mdata.size());

  //send_impl wants us to be valid before we are sent, that way it knows
  //that we are in a good state. This allows it to determine if it can forward
  //itself to different sockets.
  this->Valid = this->send_impl(socket, mode);
}

//----------------------------------------------------------------------------
//creates a job message with no data
Message::Message(remus::common::MeshIOType mtype,
                 remus::SERVICE_TYPE stype,
                 zmq::socket_t* socket,
                 SendMode mode):
  MType(mtype),
  SType(stype),
  Valid(true), //need to be initially valid to be sent
  Storage()
{
  //send_impl wants us to be valid before we are sent, that way it knows
  //that we are in a good state. This allows it to determine if it can forward
  //itself to different sockets.
  this->Valid = this->send_impl(socket, mode);
}

//----------------------------------------------------------------------------
//creates a job message from reading in the socket
Message::Message(zmq::socket_t* socket):
  MType(),
  SType(),
  Valid(false),
  Storage( boost::make_shared<zmq::message_t>() )
  {
  //we are receiving a multi part message
  //frame 0: REQ header / attachReqHeader does this
  //frame 1: Mesh Type
  //frame 2: Service Type
  //frame 3: Job Data //optional
  zmq::more_t more;
  size_t more_size = sizeof(more);
  socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);

  //construct a job message from the socket
  const bool removedHeader = zmq::removeReqHeader(*socket, ZMQ_DONTWAIT);
  bool readMeshType = false;
  bool readServiceType = false;
  bool readStorageData = false;
  bool haveStorageData = false; //states we should have the optional storage data

  //we need to decode the MType using a string buffer
  if(removedHeader)
    {
    zmq::message_t meshIOType;
    readMeshType = zmq::recv_harder(*socket, &meshIOType, ZMQ_DONTWAIT);
    if(readMeshType)
      {
      std::string bufferData(reinterpret_cast<const char*>(meshIOType.data()),
                             meshIOType.size());

      std::istringstream buffer(bufferData);
      buffer >> this->MType;
      }
    }

  //we recv the mesh type properly now try to recv the socket type
  if(readMeshType)
    {
    zmq::message_t servType;
    readServiceType = zmq::recv_harder(*socket, &servType, ZMQ_DONTWAIT);
    if(readServiceType)
      {
      this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));
      }
    }

  //now that we read recv service we can try for storage data
  if(readServiceType && readMeshType)
    {
    socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
    haveStorageData = (more > 0);
    if(haveStorageData)
      {
      //if we have a need for storage construct it now
      readStorageData = zmq::recv_harder(*socket,
                                         this->Storage.get(),
                                         ZMQ_DONTWAIT);
      }
    }

  //see if we have more data. If so we need to say we are invalid
  //as we parsed the wrong type of message
  socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
  const bool haveNothingElseToRead = (more==0)?true:false;

  if(haveStorageData)
    {
    //the transitive nature of the reads mean that if we have optional
    //storage, we only care about readStorageData and haveNothingElseToRead
    this->Valid = readStorageData && haveNothingElseToRead;
    }
  else
    {
    //the transitive nature of the reads mean that if we don't have optional
    //storage, we only care about readServiceType and haveNothingElseToRead
    this->Valid  = readServiceType && haveNothingElseToRead;
    }
  }

//------------------------------------------------------------------------------
Message& Message::operator=(Message&& other)
{
  if (this != &other)
  {
    this->MType = other.MType;
    this->SType = other.SType;
    this->Valid = other.Valid;
    this->Storage = other.Storage;
    other.Storage.reset();
  }
  return *this;
}

//------------------------------------------------------------------------------
const char* Message::data() const
{
  return this->Storage ? static_cast<char*>(this->Storage->data()) : NULL;
}

//------------------------------------------------------------------------------
std::size_t Message::dataSize() const
{
  return this->Storage ? this->Storage->size() : std::size_t(0);
}

//------------------------------------------------------------------------------
bool Message::send_impl(zmq::socket_t *socket, SendMode mode) const
{
  int flags = 0;
  if(mode == Message::NonBlocking)
    {
    flags = ZMQ_DONTWAIT;
    }

  //we are sending our selves as a multi part message
  //frame 0: REQ header / attachReqHeader does this
  //frame 1: Mesh Type
  //frame 2: Service Type
  //frame 3: Job Data //optional

  //we have to be valid to be sent
  if(!this->isValid())
    {
    return false;
    }

  const bool attached_header = zmq::attachReqHeader(*socket,flags);

  bool valid = attached_header;

  //we need to encode the MType as a string buffer
  //I worry about the performance cost of this
  std::ostringstream buffer;
  buffer << this->MType;
  std::string bufferData = buffer.str();

  zmq::message_t meshIOType(bufferData.size());
  std::memcpy(meshIOType.data(),bufferData.c_str(),bufferData.size());

  valid = valid && zmq::send_harder(*socket,meshIOType,flags|ZMQ_SNDMORE);

  zmq::message_t service(sizeof(this->SType));
  std::memcpy(service.data(),&this->SType,sizeof(this->SType));
  if(this->dataSize()> 0 && valid)
    {
    //send the service line not as the last line
    valid = zmq::send_harder(*socket,service,flags|ZMQ_SNDMORE);

    valid = valid && zmq::send_harder(*socket, *this->Storage, flags);
    }
  else if(valid) //we are done
    {
    valid = zmq::send_harder(*socket,service,flags);
    }

  return valid;
}

}
}
