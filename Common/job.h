/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_job_h
#define __meshserver_job_h

#include "stdint.h"
#include <zmq.hpp>
#include "zmqHelper.h"
#include "meshServerGlobals.h"

namespace meshserver{



class jobMessage
{
public:
  typedef unsigned int UUID_TYPE;
  jobMessage(MESH_TYPE mtype, SERVICE_TYPE stype, UUID_TYPE jobId,
             const char* data, int size):
    MType(mtype),
    SType(stype),
    JobId(jobId),
    Data(data),
    Size(size),
    ValidMsg(true),
    Storage()
    {
    }

  jobMessage(MESH_TYPE mtype, SERVICE_TYPE stype, UUID_TYPE jobId):
    MType(mtype),
    SType(stype),
    JobId(jobId),
    Data(NULL),
    Size(0),
    ValidMsg(true),
    Storage()
    {
    }

  jobMessage(zmq::socket_t& socket);

  bool send(zmq::socket_t& socket) const;
  void releaseData() { this->Data = NULL; }

  const meshserver::MESH_TYPE& meshType() const { return MType; }
  const meshserver::SERVICE_TYPE& serviceType() const { return SType; }
  const UUID_TYPE& jobId() const { return JobId; }

  const char* const data() const { return Data; }
  int dataSize() const { return Size; }

private:
  //a special struct that holds any space we need malloced
  struct DataStorage
  {
  DataStorage(): Size(0), Space(NULL){}
  DataStorage(unsigned int size): Size(size)
  {
    this->Space = new char[this->Size];
  }

  //make the assignment operator do a swap
  //and destory of the old space ( the destory is done by the fact that other
  // is temporary)
  DataStorage& operator = (DataStorage other)
  {
    //swap the size
    int s = other.Size;
    other.Size = this->Size;
    this->Size = s;

    //swap the Data pointers
    char* ptr = other.Space;
    other.Space = this->Space;
    this->Space = ptr;
  }

  ~DataStorage(){if(Size>0){delete this->Space;}}
  int Size;
  char* Space;
  };

  meshserver::MESH_TYPE MType;
  meshserver::SERVICE_TYPE SType;
  UUID_TYPE JobId;
  const char* Data;
  int Size;
  bool ValidMsg; //tells if the message is valid, mainly used by the server
  DataStorage Storage;


};

//------------------------------------------------------------------------------
jobMessage::jobMessage(zmq::socket_t &socket)
{
  //construct a job message from the socket

  //get the client tag and ignore it as we don't need it currently
  //it becomes important when we starting implementing routing and
  //requests in the broker
  {
    zmq::message_t message;
    socket.recv(&message, 0);
  }

  zmq::message_t meshType;
  socket.recv(&meshType);
  this->MType = *(reinterpret_cast<MESH_TYPE*>(meshType.data()));

  zmq::message_t servType;
  socket.recv(&servType);
  this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));

  zmq::message_t jobId;
  socket.recv(&jobId);
  this->JobId = *(reinterpret_cast<UUID_TYPE*>(jobId.data()));

  int64_t more;
  size_t more_size = sizeof(more);
  socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  if(more>0)
    {
    zmq::message_t data;
    socket.recv(&data);
    this->Size = data.size();
    this->Storage = DataStorage(this->Size);
    memcpy(this->Storage.Space,data.data(),this->Size);
    this->Data = this->Storage.Space;
    }
  else
    {
    this->Size = 0;
    this->Data = NULL;
    }

  //see if we have more data. If so we need to say we are invalid
  //as we parsed the wrong type of message
  socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  ValidMsg=(more==0)?false:true;
}

//------------------------------------------------------------------------------
bool jobMessage::send(zmq::socket_t &socket) const
  {
  //we are sending our selves as a multi part message
  //frame 0: empty ( REQ emultion )
  //frame 1: "MDP/Client" //for now we are going to emulate major domo pattern
  //frame 2: Mesh Type
  //frame 2: Service Type
  //frame 3: Job Id
  //frame 4: Job Data //optional

  if(!this->ValidMsg)
    {
    return false;
    }

  zmq::message_t reqEmu(0);
  socket.send(reqEmu,ZMQ_SNDMORE);

  zmq::message_t clientTag(meshserver::ClientTag.size());
  memcpy(clientTag.data(),meshserver::ClientTag.data(),meshserver::ClientTag.size());
  socket.send(clientTag,ZMQ_SNDMORE);

  zmq::message_t meshType(sizeof(this->MType));
  memcpy(meshType.data(),&this->MType,sizeof(this->MType));
  socket.send(meshType,ZMQ_SNDMORE);

  zmq::message_t service(sizeof(this->SType));
  memcpy(service.data(),&this->SType,sizeof(this->SType));
  socket.send(service,ZMQ_SNDMORE);

  zmq::message_t jobId(sizeof(this->JobId));
  memcpy(jobId.data(),&this->JobId,sizeof(this->JobId));
  socket.send(jobId,ZMQ_SNDMORE);

  if(this->Size > 0)
    {
    zmq::message_t data(this->Size);
    memcpy(data.data(),this->Data,this->Size);
    socket.send(data);
    }

  return true;
  }

//------------------------------------------------------------------------------
}

#endif
