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
class jobStatus
{
  //a multipart message the encodes
  //which job the status is for,
  //and what state the job is in

};

class job
{
  typedef unsigned int UUID_TYPE;

  //this class currently copies the client side data into
  //the zero MQ message. It currently doesn't support zero copy,
  //but it could in the future.
public:
  job(meshserver::MESH_TYPES type,
      const char* data, unsigned int dataSize):
    MeshType(type),
    Data(data),
    Size(dataSize),
    Id(NextId++)
  {
  }

bool send(zmq::socket_t& socket) const
  {
  //we are sending our selves as a multi part message
  //frame 0: empty ( REQ emultion )
  //frame 1: "MDP/Client" //for now we are going to emulate major domo pattern
  //frame 2: Service ( mesh domain type ) // aka service
  //frame 3: Job Id
  //frame 4: Job Data

  zmq::message_t reqEmu(0);
  socket.send(reqEmu,ZMQ_SNDMORE);

  zmq::message_t clientTag(meshserver::ClientTag.size());
  memcpy(clientTag.data(),meshserver::ClientTag.data(),meshserver::ClientTag.size());
  socket.send(clientTag,ZMQ_SNDMORE);

  zmq::message_t service(sizeof(this->MeshType));
  memcpy(service.data(),&this->MeshType,sizeof(this->MeshType));
  socket.send(service,ZMQ_SNDMORE);

  zmq::message_t jobId(sizeof(this->Id));
  memcpy(jobId.data(),&this->Id,sizeof(this->Id));
  socket.send(jobId,ZMQ_SNDMORE);

  zmq::message_t data(this->Size);
  memcpy(data.data(),this->Data,this->Size);
  socket.send(data);
  }

private:
  const meshserver::MESH_TYPES MeshType;
  const char* const Data;
  const int Size;

  const UUID_TYPE Id;
  static UUID_TYPE NextId;
};

//define the first id
unsigned int meshserver::job::NextId = 0;
}

#endif
