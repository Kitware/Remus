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

#include <remus/server/detail/uuidHelper.h>
#include <remus/testing/Testing.h>

#include <remus/proto/zmqHelper.h>

int UnitTestUUIDHelper(int, char *[])
{
  //pretty simple to test.
  const boost::uuids::uuid example = remus::testing::UUIDGenerator();

  const std::string text = remus::to_string(example);
  const std::string b_text = boost::lexical_cast<std::string>(example);
  REMUS_ASSERT( (text==b_text) );

  boost::uuids::uuid r_from_str = remus::to_uuid(b_text);
  boost::uuids::uuid b_from_str =
                          boost::lexical_cast<boost::uuids::uuid>(b_text);
  REMUS_ASSERT( (r_from_str==b_from_str) );

  zmq::context_t context(0);
  zmq::socket_t in_socket(context, ZMQ_PAIR);
  zmq::socket_t out_socket(context, ZMQ_PAIR);

  zmq::socketInfo<zmq::proto::inproc> channel(remus::testing::UniqueString());
  zmq::bindToAddress(in_socket, channel);
  zmq::connectToAddress(out_socket, channel);

  //send the message
  remus::common::MeshIOType type;
  remus::proto::Message in_msg = remus::proto::send_Message(type,
                                                            remus::CAN_MESH_IO_TYPE,
                                                            text,
                                                            &in_socket);
  REMUS_ASSERT( (in_msg.isValid()) );

  remus::proto::Message out_msg = remus::proto::receive_Message(&out_socket);
  REMUS_ASSERT( (out_msg.isValid()) );

  //message data received needs to be an uuid.
  boost::uuids::uuid msg_uuid = remus::to_uuid(out_msg);
  REMUS_ASSERT( (r_from_str==msg_uuid) );

  return 0;
}
