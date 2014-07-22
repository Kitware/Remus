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

#include <boost/make_shared.hpp>

namespace remus{
namespace proto{

//----------------------------------------------------------------------------
Message::Message(remus::common::MeshIOType mtype, remus::SERVICE_TYPE stype,
                 const std::string& mdata):
  MType(mtype),
  SType(stype),
  ValidMsg(true),
  Storage( boost::make_shared<zmq::message_t>(mdata.size()) )
{
  std::memcpy(Storage->data(),mdata.data(),mdata.size());
}

//----------------------------------------------------------------------------
//pass in a data pointer that the message will use when sending
//the pointer data can't become invalid before you call send.
Message::Message(remus::common::MeshIOType mtype, remus::SERVICE_TYPE stype,
                 const char* mdata, std::size_t size):
  MType(mtype),
  SType(stype),
  ValidMsg(true),
  Storage( boost::make_shared<zmq::message_t>(size) )
{
  std::memcpy(this->Storage->data(),mdata,size);
}

//----------------------------------------------------------------------------
//creates a job message with no data
Message::Message(remus::common::MeshIOType mtype, remus::SERVICE_TYPE stype):
  MType(mtype),
  SType(stype),
  ValidMsg(true),
  Storage()
{
}

//----------------------------------------------------------------------------
//creates a job message from reading in the socket
Message::Message(zmq::socket_t* socket)
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
  const bool removedHeader = zmq::removeReqHeader(*socket);
  bool readMeshType = false;
  bool readServiceType = false;
  bool readStorageData = false;
  bool haveStorageData = false; //states we should have the optional storage data

  //we need to decode the MType using a string buffer
  if(removedHeader)
    {
    zmq::message_t meshIOType;
    readMeshType = zmq::recv_harder(*socket,&meshIOType);
    if(readMeshType)
      {
      std::string bufferData(reinterpret_cast<const char*>(meshIOType.data()),
                             meshIOType.size());

      std::istringstream bufferData(bufferData);
      std::string in_type, out_type;
      buffer >> in_type;
      buffer >> out_type;
      this->MType = remus::common::MeshIOType(in_type,out_type);
      }
    }

  //we recv the mesh type properly now try to recv the socket type
  if(readMeshType)
    {
    zmq::message_t servType;
    readServiceType = zmq::recv_harder(*socket, &servType);
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
      this->Storage = boost::make_shared<zmq::message_t>();
      readStorageData = zmq::recv_harder(*socket, this->Storage.get());
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
    this->ValidMsg = readStorageData && haveNothingElseToRead;
    }
  else
    {
    //the transitive nature of the reads mean that if we don't have optional
    //storage, we only care about readServiceType and haveNothingElseToRead
    this->ValidMsg  = readServiceType && haveNothingElseToRead;
    }
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
bool Message::send(zmq::socket_t *socket) const
{
  return this->send_impl(socket);
}

//------------------------------------------------------------------------------
bool Message::sendNonBlocking(zmq::socket_t *socket) const
{
  return this->send_impl(socket,ZMQ_DONTWAIT);
}

//------------------------------------------------------------------------------
bool Message::send_impl(zmq::socket_t *socket, int flags) const
{

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

  zmq::attachReqHeader(*socket,flags);

  bool valid = true;

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


    valid = valid && zmq::send_harder(*socket,
                                      *this->Storage.get(),
                                      flags);
    }
  else if(valid) //we are done
    {
    valid = zmq::send_harder(*socket,service,flags);
    }

  return valid;
}

}
}
