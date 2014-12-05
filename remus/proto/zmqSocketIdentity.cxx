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

#include <algorithm>

#include <remus/common/MD5Hash.h>

#ifdef _MSC_VER
# pragma warning(push)
//disable warning about using std::copy with pointers
# pragma warning(disable: 4996)
#endif

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{

//------------------------------------------------------------------------------
SocketIdentity::SocketIdentity(const char* d, std::size_t s):
    Size(s)
  {
    std::copy(d,d+s,this->Data);
  }

//------------------------------------------------------------------------------
SocketIdentity::SocketIdentity():
  Size(0)
{

}

//------------------------------------------------------------------------------
bool SocketIdentity::operator ==(const SocketIdentity& b) const
{
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

//------------------------------------------------------------------------------
std::string to_string(const zmq::SocketIdentity& add)
{
  return remus::common::MD5Hash(add.data(),add.size());
}

}

//reset our warnings to the original level
#ifdef _MSC_VER
# pragma warning(pop)
#endif
