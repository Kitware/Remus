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

#ifndef __remus_common_zeroTraits_h
#define __remus_common_zeroTraits_h

#include <string>

namespace zmq
{
namespace proto
{
//basic transport type tags, used by socketInfo
//should these
struct tcp{};
struct ipc{};
struct inproc{};
}
inline std::string to_string(const zmq::proto::tcp& t)
  {
  return "tcp://";
  }
inline std::string to_string(const zmq::proto::ipc& t)
  {
  return "ipc://";
  }
inline std::string to_string(const zmq::proto::inproc& t)
  {
  return "inproc://";
  }
}

#endif // __remus_common_zeroTraits_h
