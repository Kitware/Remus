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

#include <iostream>
#include <string>
#include <vector>

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
  buffer.rdbuf()->sgetn(&msg[0],msg.size());
}

//------------------------------------------------------------------------------
template<typename BufferType>
inline std::string extractString(BufferType& buffer, int size)
{
  std::vector<char> msg(size);
  extractVector(buffer,msg);
  return std::string(&msg[0],size);
}


//------------------------------------------------------------------------------
template<typename BufferType>
inline void writeString(BufferType& buffer, const std::string& str)
{
  buffer.rdbuf()->sputn(str.c_str(),str.length());
  buffer << std::endl;
}


//------------------------------------------------------------------------------
template<typename BufferType>
inline void writeString(BufferType& buffer, const char * str, std::size_t size)
{
  buffer.rdbuf()->sputn(str,size);
  buffer << std::endl;
}

}
}

#endif