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
inline std::string scheme_name(zmq::proto::tcp)
  {
  return "tcp";
  }
inline std::string scheme_name(zmq::proto::ipc)
  {
  return "ipc";
  }
inline std::string scheme_name(zmq::proto::inproc)
  {
  return "inproc";
  }

inline std::string separator(zmq::proto::tcp)
  {
  return "://";
  }
inline std::string separator(zmq::proto::ipc)
  {
  return "://";
  }
inline std::string separator(zmq::proto::inproc)
  {
  return "://";
  }

inline std::string scheme_and_separator(zmq::proto::tcp)
  {
  return "tcp://";
  }
inline std::string scheme_and_separator(zmq::proto::ipc)
  {
  return "ipc://";
  }
inline std::string scheme_and_separator(zmq::proto::inproc)
  {
  return "inproc://";
  }


}

#endif // __remus_common_zeroTraits_h
