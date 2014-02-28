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
#ifndef remus_proto_zmqSocketIdentity_h
#define remus_proto_zmqSocketIdentity_h

#include <algorithm>
#include <cstddef>
#include <string>

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{
//holds the identity of a zero mq socket in a way that is
//easier to copy around
struct SocketIdentity
{
  SocketIdentity(const char* data, std::size_t size):
    Size(size)
    {
    std::copy(data,data+size,Data);
    }

  SocketIdentity():
    Size(0)
    {}

  bool operator ==(const SocketIdentity& b) const
  {
    if(this->size() != b.size()) { return false; }
    return std::equal(this->data(),this->data()+this->size(),b.data());
  }

  bool operator<(const SocketIdentity& b) const
  {
    //sort first on size
    if(this->Size != b.size()) { return this->Size < b.size(); }

    //second sort on content.
    return std::lexicographical_compare(this->data(),this->data()+this->size(),
                                        b.data(), b.data()+b.size());
  }

  const char* data() const { return &Data[0]; }
  std::size_t size() const { return Size; }

private:
  std::size_t Size;
  char Data[256];
};

inline std::string to_string(const zmq::SocketIdentity& add)
{
  return std::string(add.data(),add.size());
}

}

#endif // remus_proto_zmqSocketIdentity_h
