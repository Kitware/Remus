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

#include <cstddef>
#include <string>

//included for export symbols
#include <remus/proto/ProtoExports.h>

#include <remus/common/CompilerInformation.h>
#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{
//holds the identity of a zero mq socket in a way that is
//easier to copy around
struct REMUSPROTO_EXPORT SocketIdentity
{
  SocketIdentity(const char* d, std::size_t s);

  SocketIdentity();

  bool operator ==(const SocketIdentity& b) const;

  bool operator<(const SocketIdentity& b) const;

  const char* data() const { return &Data[0]; }
  std::size_t size() const { return Size; }

  //returns this socket identity as a human
  //readable name
  const std::string& name() const { return this->Name; }

private:
  std::size_t Size;
  char Data[256];
  std::string Name;
};

}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif

#endif // remus_proto_zmqSocketIdentity_h
