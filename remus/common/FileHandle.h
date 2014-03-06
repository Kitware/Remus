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

#ifndef remus_common_FileHandle_h
#define remus_common_FileHandle_h

#include <string>

namespace remus {
namespace common {

struct FileHandle
{
  //construct a lightweight wrapper to signify that the string is a file path
  explicit FileHandle(const std::string& path):
    Path(path)
    { }

  std::size_t size() const { return Path.size(); }
  const char* data() const { return Path.data(); }

  const std::string& path() const{ return Path; }
private:
  std::string Path;
};

}
}

#endif
