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

//For historical reasons, the Windows.h header defaults to including the
//Winsock.h header file for Windows Sockets 1.1. The declarations in the
//Winsock.h header file will conflict with the declarations in the Winsock2.h
//header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN define
//prevents the Winsock.h from being included by the Windows.h header.
# ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
# endif
#include <windows.h>

namespace  remus {
namespace  common {
namespace  detail {

//------------------------------------------------------------------------------
boost::filesystem::path getOSExecLocation()
{
  boost::filesystem::path execLoc;

  //GetModuleFileName doesn't always report the correct errors when the
  //path would require the entire buffer. So to work around this issue
  //we are going to create a buffer twice the size of MAX_PATH and then
  //increase it as needed.
  std::size_t factor = 2;
  std::size_t base_size = MAX_PATH; //as defined by windows.h


  bool found = false;
  while(!found)
    {
    std::size_t size = base_size * factor;

    //+1 size to handle a null term character
    wchar_t* buffer = new wchar_t[size + 1];
    LPWSTR ptr = buffer;
    DWORD len = GetModuleFileNameW(NULL, ptr, static_cast<DWORD>(size) );
    buffer[size] = 0;

    if(len == 0 || factor > 5)
      {
      //failure case, we need to return an empty path. We either
      //can't locate a path or the path is crazzzzzzzzzzzy long
      found = true;
      }
    else if(len <= size)
      {
      //we are done, drop the executable name, make this path absolute, and
      //canonical
      found = true;

      boost::filesystem::path p(buffer);
      p = p.parent_path();
      execLoc = boost::filesystem::canonical(p);
      }

    delete[] buffer;
    factor++; //increase the buffer size for our next attempt

    }

  return execLoc;
}

}
}
}
