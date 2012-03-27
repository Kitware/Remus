/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_job_h
#define __meshserver_job_h

#include <boost/uuid/uuid.hpp>
#include <boost/shared_ptr.hpp>
#include <zmq.hpp>

#include <cstddef>
#include "zmqHelper.h"
#include "meshServerGlobals.h"

#include <iostream>

namespace meshserver{
class JobMessage
{
public:
  JobMessage(MESH_TYPE mtype, SERVICE_TYPE stype, const char* data, int size);
  JobMessage(MESH_TYPE mtype, SERVICE_TYPE stype);
  JobMessage(zmq::socket_t& socket);

  bool send(zmq::socket_t& socket) const;
  void releaseData() { this->Data = NULL; }

  const meshserver::MESH_TYPE& meshType() const { return MType; }
  const meshserver::SERVICE_TYPE& serviceType() const { return SType; }

  const char* const data() const { return Data; }
  int dataSize() const { return Size; }

  bool isValid() const { return ValidMsg; }

  template<typename T>
  void dump(T& t) const;

private:
  //a special struct that holds any space we need malloced
  struct DataStorage
    {
    DataStorage(): Size(0), Space(NULL){}
    DataStorage(unsigned int size): Size(size){this->Space = new char[this->Size];}
    ~DataStorage(){if(Size>0){delete this->Space;}}
    int Size;
    char* Space;
    };

  meshserver::MESH_TYPE MType;
  meshserver::SERVICE_TYPE SType;
  const char* Data;
  int Size;
  bool ValidMsg; //tells if the message is valid, mainly used by the server

  boost::shared_ptr<DataStorage> Storage;
};

//------------------------------------------------------------------------------
JobMessage::JobMessage(MESH_TYPE mtype, SERVICE_TYPE stype, const char* data, int size):
  MType(mtype),
  SType(stype),
  Data(data),
  Size(size),
  ValidMsg(true),
  Storage(new DataStorage())
  {
  }

//------------------------------------------------------------------------------
JobMessage::JobMessage(MESH_TYPE mtype, SERVICE_TYPE stype):
  MType(mtype),
  SType(stype),
  Data(NULL),
  Size(0),
  ValidMsg(true),
  Storage(new DataStorage())
  {
  }

//------------------------------------------------------------------------------
JobMessage::JobMessage(zmq::socket_t &socket)
{
  //construct a job message from the socket
  zmq::stripSocketSig(socket);

  zmq::message_t meshType;
  socket.recv(&meshType);
  this->MType = *(reinterpret_cast<MESH_TYPE*>(meshType.data()));

  zmq::message_t servType;
  socket.recv(&servType);
  this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));

  int64_t more;
  size_t more_size = sizeof(more);
  socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  if(more>0)
    {
    zmq::message_t data;
    socket.recv(&data);
    this->Size = data.size();
    this->Storage.reset(new DataStorage(this->Size));
    memcpy(this->Storage->Space,data.data(),this->Size);
    this->Data = this->Storage->Space;
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

//------------------------------------------------------------------------------
bool JobMessage::send(zmq::socket_t &socket) const
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

  zmq::message_t meshType(sizeof(this->MType));
  memcpy(meshType.data(),&this->MType,sizeof(this->MType));
  socket.send(meshType,ZMQ_SNDMORE);

  zmq::message_t service(sizeof(this->SType));
  memcpy(service.data(),&this->SType,sizeof(this->SType));
  if(this->Size> 0)
    {
    //send the service line not as the last line
    socket.send(service,ZMQ_SNDMORE);

    //send the data
    zmq::message_t data(this->Size);
    memcpy(data.data(),this->Data,this->Size);
    socket.send(data);
    }
  else //we are done
    {
    socket.send(service);
    }
  return true;
  }

//------------------------------------------------------------------------------
template<typename T>
void JobMessage::dump(T& t) const
  {
  //dump the info to the t stream
  t << "Valid: " << this->isValid() << std::endl;
  t << "Mesh Type: " << this->meshType() << std::endl;
  t << "Serivce Type: " << this->serviceType() << std::endl;
  t << "Size: " << this->dataSize() << std::endl;
  if(this->dataSize() > 0)
    {
    t << "Data: " << std::string(this->Data,this->dataSize()) << std::endl;
    }
  }

//------------------------------------------------------------------------------
boost::uuids::uuid to_uuid(const meshserver::JobMessage& msg)
{
  //take the contents of the msg and convert it to an uuid
  //no type checking will be done to make sure this is valid for now
  boost::uuids::uuid id;
  memcpy(&id,msg.data(),16); //boost uuid is always 16 characters long
}

}

#endif
