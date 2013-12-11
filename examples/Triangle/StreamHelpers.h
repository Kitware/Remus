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

#include <vector>

#ifndef __helpers_StreamHelpers_h
#define __helpers_StreamHelpers_h

namespace helpers
{
//----------------------------------------------------------------------------
template<typename T>
inline bool AllocFromStream(std::stringstream& buffer, std::vector<T> &dest,
                            int numElements)
{
if(numElements <= 0)
  {
  dest.resize(0);
  return false;
  }

//first we alloc dest;
dest.resize(numElements);

//now we fill it from the buffer
char* cdest = reinterpret_cast<char*>(&dest[0]);
const std::streamsize size = sizeof(T)*numElements;

//strip away the new line character at the start
if(buffer.peek()=='\n')
  {buffer.get();}

const std::streamsize readLen = buffer.rdbuf()->sgetn(cdest,size);
bool valid = readLen == size;
return valid;
}

//----------------------------------------------------------------------------
//uses the vector size for how much to write
template<typename T>
inline bool WriteToStream(std::stringstream& buffer, const std::vector<T>& src)
{
if(src.size() == 0)
  {return false;}

//now we fill it from the buffer
const char* csrc = reinterpret_cast<const char*>(&src[0]);
const std::streamsize size = sizeof(T)*src.size();
buffer.write(csrc,size);
buffer << std::endl;
return !buffer.bad();
}

//----------------------------------------------------------------------------
//uses the vector size for how much to write
template<typename T>
inline bool WriteToStream(std::stringstream& buffer, T* src,
                          int numElements)
{
if(numElements == 0)
  {return false;}

//now we fill it from the buffer
const char* csrc = reinterpret_cast<const char*>(&src);
const std::streamsize size = sizeof(T)*numElements;
buffer.write(csrc,size);
buffer << std::endl;
return !buffer.bad();
}

}
#endif
