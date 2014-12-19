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

#include <boost/cstdint.hpp>

//suppress warnings inside boost headers for gcc, clang and MSVC
#ifndef _MSC_VER
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <boost/lexical_cast.hpp>

#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <algorithm>

#include <remus/proto/zmq.hpp>

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

}

namespace zmq
{

//------------------------------------------------------------------------------
SocketIdentity::SocketIdentity( const char* start, std::size_t size )
{
  this->Size = size;
  std::memcpy(this->Data, start, size);

  if(this->Data[0] == '\0' && this->Size == 5 )
    {
    //by default zmq reserves all id's that start with '\0', and they use an unsigned 32 bit
    //int in big endian form to represent the identity of the socket. If we detect this we convert this to a human
    //readable name
    boost::uint32_t integerId = convertBEToUnsignedInteger(this->Data);
    this->Name = boost::lexical_cast<std::string>( integerId );
    }
  else
    {
    //somebody has set a custom socket identity so we need to respect that
    this->Name = std::string(start,this->Size);
    }

}

//------------------------------------------------------------------------------
SocketIdentity::SocketIdentity():
Size(0),
Data(),
Name()
{
}

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
