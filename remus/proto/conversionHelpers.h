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

#ifndef remus_proto_conversionHelpers_h
#define remus_proto_conversionHelpers_h

#include <string>
#include <ostream>
#include <vector>

#include <cassert>

namespace remus {
namespace internal
{
//------------------------------------------------------------------------------
template<typename BufferType>
inline void extractVector(BufferType& buffer, std::vector<char>& msg)
{
  if(buffer.peek()=='\n')
    {
    buffer.get();
    }
  if(msg.size() > 0)
    {
    const std::streamsize readLen = buffer.rdbuf()->sgetn(&msg[0],msg.size());
    assert(readLen == static_cast<std::streamsize>(msg.size()));
    }
}

//------------------------------------------------------------------------------
template<typename BufferType>
inline std::string extractString(BufferType& buffer, int size)
{
  std::vector<char> msg(size);
  extractVector(buffer,msg);
  if(size > 0)
    {
    return std::string(&msg[0],size);
    }
  else
    {
    return std::string();
    }

}

//------------------------------------------------------------------------------
template<typename BufferType>
inline bool writeString(BufferType& buffer, const char * str, std::size_t size)
{
  const std::streamsize writeLen = buffer.rdbuf()->sputn(str,size);
  buffer << std::endl;
  assert(writeLen == static_cast<std::streamsize>(size));
  return writeLen == static_cast<std::streamsize>(size);
}

//------------------------------------------------------------------------------
template<typename BufferType>
inline bool writeString(BufferType& buffer, const std::string& str)
{
  return writeString(buffer,str.c_str(),str.size());
}


}
}

#endif
