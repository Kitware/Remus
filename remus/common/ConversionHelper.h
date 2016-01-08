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

#ifndef remus_common_ConversionHelper_h
#define remus_common_ConversionHelper_h

#include <string>
#include <ostream>
#include <vector>

#include <cassert>

namespace remus {
namespace internal
{
//------------------------------------------------------------------------------
//requires the msg to be allocated before calling, as we will extract it's size
//from the buffer. If msg size is 0 we will extract nothing
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
    (void) readLen;
    }
}

//------------------------------------------------------------------------------
//requires the msg_data to be allocated to at least msg_size before calling,
//as we will extract msg_size characters from the buffer.
//if msg_data is NULL or msg_size is 0 we will extract nothing
template<typename BufferType>
inline void extractArray(BufferType& buffer, char* msg_data,
                         std::size_t msg_size)
{
  if(buffer.peek()=='\n')
    {
    buffer.get();
    }
  if(msg_size > 0 && msg_data != NULL)
    {
    const std::streamsize readLen = buffer.rdbuf()->sgetn(msg_data,msg_size);
    assert(readLen == static_cast<std::streamsize>(msg_size));
    (void) readLen;
    }
}

//------------------------------------------------------------------------------
template<typename BufferType>
inline std::string extractString(BufferType& buffer, std::size_t size)
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
