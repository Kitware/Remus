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

#include <boost/uuid/random_generator.hpp>


int UnitTestUUIDHelper(int, char *[])
{
  boost::uuids::random_generator generator;

  //pretty simple to test.
  const boost::uuids::uuid example = generator();

  const std::string text = remus::to_string(example);
  const std::string b_text = boost::lexical_cast<std::string>(example);
  REMUS_ASSERT( (text==b_text) );

  boost::uuids::uuid r_from_str = remus::to_uuid(b_text);
  boost::uuids::uuid b_from_str =
                          boost::lexical_cast<boost::uuids::uuid>(b_text);
  REMUS_ASSERT( (r_from_str==b_from_str) );

  remus::common::MeshIOType type;
  remus::common::Message msg(type, remus::CAN_MESH, text);

  //message data needs to be an uuid.
  boost::uuids::uuid msg_uuid = remus::to_uuid(msg);
  REMUS_ASSERT( (r_from_str==msg_uuid) );

  return 0;
}
