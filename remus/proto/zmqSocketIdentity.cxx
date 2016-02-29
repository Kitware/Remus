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

#include <remus/common/CompilerInformation.h>
#include <remus/proto/zmq.hpp>

//suppress warnings inside boost headers for gcc, clang and MSVC
REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <algorithm>
#include <iostream>
#include <ios>

namespace
{
boost::uint32_t  convertBEToUnsignedInteger(const char buffer[256])
{
  //convert from big endian encoding of an integer to the host integer. We skip
  //buffer[0] as that is holding the null byte signifiying that this is a zmq be uint32
  return static_cast<boost::uint32_t>(buffer[1]) << 24 |
              static_cast<boost::uint32_t>(buffer[2]) << 16 |
              static_cast<boost::uint32_t>(buffer[3]) << 8   |
              static_cast<boost::uint32_t>(buffer[4]);
}

std::string convertDigestToHex(const char* input, std::size_t s)
{
  //presumes that s is divisible by 4!
  const int* t = reinterpret_cast<const int*>(input);
  const int size = s/4;
  std::stringstream buffer;
  buffer << std::hex << t[0] << "-";
  buffer << std::hex << t[1] << "-";
  buffer << std::hex << t[2] << "-";
  buffer << std::hex << t[3];
  return buffer.str();
}


}

namespace zmq
{

//------------------------------------------------------------------------------
SocketIdentity::SocketIdentity( const char* start, std::size_t s )
{
  this->Size = s;
  std::memcpy(this->Data, start, s);


  if(this->Data[0] == '\0' && this->Size == 5 )
    {
    //this is most likely the new 4byte integer count from zmq 3.0+
    //by default zmq reserves all id's that start with '\0', and they use an unsigned 32 bit
    //int in big endian from to represent the identity of the socket. If we detect this we convert this to a human
    //readable name
    boost::uint32_t integerId = convertBEToUnsignedInteger(this->Data);
    this->Name = boost::lexical_cast<std::string>( integerId );
    }
 else if(this->Data[0] == '\0' && this->Size == 17 )
    {
    //this is most likely the old uuid generated version from zmq 2.0
    //so lets us a slower path to generate the uuid
    this->Name = convertDigestToHex(this->Data+1, this->Size-1);
    }
  else
    {
    //somebody has set a custom socket identity so we need to respect that
    this->Name = std::string(start,this->Size);
    }

}

//disable warning about elements of array 'Data' will be default initialized
//this is only a warning on msvc, since previously it was broken and wouldn't
//default initialize member arrays
# ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4351)
# endif
//------------------------------------------------------------------------------
SocketIdentity::SocketIdentity():
Size(0),
Data(),
Name()
{
}
//reset our warnings to the original level
# ifdef _MSC_VER
#   pragma warning(pop)
# endif

//------------------------------------------------------------------------------
bool SocketIdentity::operator ==(const SocketIdentity& b) const
{
  //this can't just compare names, as the names are unique. You could have a user
  //encode a string of "1" as the unique id of the socket and have a zmq provided
  //id be 1 which would produce the name of "1", while the actual Data blocks aren't the same
  if(this->size() != b.size()) { return false; }
  return std::equal(this->data(),this->data()+this->size(),b.data());
}

//------------------------------------------------------------------------------
bool SocketIdentity::operator<(const SocketIdentity& b) const
{
    //sort first on size
    if(this->Size != b.size()) { return this->Size < b.size(); }

    //second sort on content.
    return std::lexicographical_compare(this->data(),this->data()+this->size(),
                                        b.data(), b.data()+b.size());
}


}
