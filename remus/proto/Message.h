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

#include <remus/common/MeshIOType.h>
#include <remus/common/remusGlobals.h>

//for export symbols
#include <remus/proto/ProtoExports.h>

namespace zmq
{
  class message_t;
  class socket_t;
}

namespace remus{
namespace proto{

//forward declare message class;
class Message;

//----------------------------------------------------------------------------
//pass in a std::string that we will copy and send.
//The message returned will have a copy of the data given to it.
REMUSPROTO_EXPORT
Message send_Message(remus::common::MeshIOType mtype,
                     remus::SERVICE_TYPE stype,
                     const std::string& data,
                     zmq::socket_t* socket);

//----------------------------------------------------------------------------
//send a message that has no data.
//The message returned will not have any data associated with it
REMUSPROTO_EXPORT
Message send_Message(remus::common::MeshIOType mtype,
                     remus::SERVICE_TYPE stype,
                     zmq::socket_t* socket);

//----------------------------------------------------------------------------
//pass in a std::string that we will copy and send.
//The message returned will have a copy of the data given to it.
REMUSPROTO_EXPORT
Message send_NonBlockingMessage(remus::common::MeshIOType mtype,
                                remus::SERVICE_TYPE stype,
                                const std::string& data,
                                zmq::socket_t* socket);

//----------------------------------------------------------------------------
//send a message that has no data.
//The message returned will not have any data associated with it
REMUSPROTO_EXPORT
Message send_NonBlockingMessage(remus::common::MeshIOType mtype,
                                remus::SERVICE_TYPE stype,
                                zmq::socket_t* socket);

//----------------------------------------------------------------------------
//parse a message from a socket
//The message returned will have data associated with if it is valid
REMUSPROTO_EXPORT
Message receive_Message( zmq::socket_t* socket );

//----------------------------------------------------------------------------
//forward a message that has been received to another socket
//
REMUSPROTO_EXPORT
bool forward_Message(const remus::proto::Message& message,
                     zmq::socket_t* socket);


//All creation of this class needs to happen through the make_Message
//set of functions
class REMUSPROTO_EXPORT Message
{
public:
  enum SendMode{Blocking=0, NonBlocking=1};

  const remus::common::MeshIOType& MeshIOType() const { return MType; }
  const remus::SERVICE_TYPE& serviceType() const { return SType; }

  //these are going to have to be removed
  const char* data() const;
  std::size_t dataSize() const;

  //is true if all the message was sent, or all the message was received.
  bool isValid() const { return Valid; }

private:
  friend Message send_Message(remus::common::MeshIOType mtype,
                              remus::SERVICE_TYPE stype,
                              const std::string& data,
                              zmq::socket_t* socket);

  friend Message send_Message(remus::common::MeshIOType mtype,
                              remus::SERVICE_TYPE stype,
                              zmq::socket_t* socket);

  friend Message send_NonBlockingMessage(remus::common::MeshIOType mtype,
                                         remus::SERVICE_TYPE stype,
                                         const std::string& data,
                                         zmq::socket_t* socket);

  friend Message send_NonBlockingMessage(remus::common::MeshIOType mtype,
                                         remus::SERVICE_TYPE stype,
                                         zmq::socket_t* socket);

  friend Message receive_Message( zmq::socket_t* socket );

  friend bool forward_Message(const remus::proto::Message& message,
                              zmq::socket_t* socket);


  //----------------------------------------------------------------------------
  //pass in a std::string that Message we will copy and send
  Message(remus::common::MeshIOType mtype,
          remus::SERVICE_TYPE stype,
          const std::string& data,
          zmq::socket_t* socket,
          SendMode mode);

  //----------------------------------------------------------------------------
  //creates a Message with no data
  Message(remus::common::MeshIOType mtype,
          remus::SERVICE_TYPE stype,
          zmq::socket_t* socket,
          SendMode mode);

  //----------------------------------------------------------------------------
  //creates a Message from reading from the socket
  explicit Message(zmq::socket_t* socket);

  bool send_impl(zmq::socket_t* socket, SendMode mode) const;

  remus::common::MeshIOType MType;
  remus::SERVICE_TYPE SType;
  bool Valid; //tells if the message is valid

  boost::shared_ptr<zmq::message_t> Storage;
};

}
}
#endif //remus_Message_h
