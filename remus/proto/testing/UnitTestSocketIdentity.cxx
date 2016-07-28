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

#include <remus/proto/zmqSocketIdentity.h>
#include <remus/proto/zmq.hpp>

#include <remus/testing/Testing.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/cstdint.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <string>
#include <set>


namespace {

std::string randomIdentity()
{
  return remus::testing::UniqueString();
}

std::string nextIntegerIdentity()
{
  static boost::uint32_t integerId;
  //encode the unsigned integer as a big endian stream
  //prefixed with a null byte
  char buffer[5];
  buffer[0] = '\0';
  buffer[1] = static_cast<unsigned char>((integerId >> 24) & 0xff);
  buffer[2] = static_cast<unsigned char>((integerId >> 16) & 0xff);
  buffer[3] = static_cast<unsigned char>((integerId >> 8) & 0xff);
  buffer[4] = static_cast<unsigned char>(integerId & 0xff);

  ++integerId;
  return std::string(buffer, 5);
}


template <typename F>
void verify_uniqueness( F f )
{

  //test to make sure that two sockets never resolve to being
  //equal to each other even when their source data is different.
  std::set< zmq::SocketIdentity > sockets;
  const std::size_t size =2048;
  for(std::size_t i=0; i < size; ++i)
    {
    std::string name = f();
    zmq::SocketIdentity si(name.c_str(), name.size());
    sockets.insert(si);
    }
    REMUS_ASSERT( (sockets.size() == size) );
}

} //namespace


int UnitTestSocketIdentity(int, char *[])
{

  //verify that null socket's are the same
  zmq::SocketIdentity empty;
  zmq::SocketIdentity empty2 = zmq::SocketIdentity();

  REMUS_ASSERT( (empty2.name() == empty.name()) );
  REMUS_ASSERT( (empty2 == empty) );
  REMUS_ASSERT( (empty2< empty) == false);

  //verify that passing in a string that doesn't start with null
  //produces a properly labelled socket identity
  std::string randomName = randomIdentity();
  zmq::SocketIdentity stringSocket(randomName.c_str(), randomName.size());

  REMUS_ASSERT( (stringSocket.name() == randomName) );
  REMUS_ASSERT( (stringSocket.size() == randomName.size()) );

  //verify passing in a zmq \0 prefixed big endian unsigned
  //integer produces the correct results
  std::string intName = nextIntegerIdentity();
  zmq::SocketIdentity intSocket(intName.c_str(), intName.size());

  REMUS_ASSERT( (intSocket.name() != intName) );
  REMUS_ASSERT( (intSocket.name() == "0") );
  REMUS_ASSERT( (intSocket.size() == 5) );
  REMUS_ASSERT( ( *intSocket.data() == '\0') );

  std::string intName2 = nextIntegerIdentity();
  zmq::SocketIdentity intSocket2(intName2.c_str(), intName2.size());

  REMUS_ASSERT( (intSocket2.name() != intName) );
  REMUS_ASSERT( (intSocket2.name() == "1") );
  REMUS_ASSERT( (intSocket2.size() == 5) );
  REMUS_ASSERT( ( *intSocket2.data() == '\0') );

  //we just need to verify that the encoding logic is correct.
  verify_uniqueness(randomIdentity);
  verify_uniqueness(nextIntegerIdentity);

  return 0;
}
