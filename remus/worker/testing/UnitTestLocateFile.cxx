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

#include <remus/worker/LocateFile.h>
#include <remus/testing/Testing.h>

#include <string>

REMUS_THIRDPARTY_PRE_INCLUDE
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace {

using namespace remus::worker;

void verify_executable_path()
{
  //Test that the path coming back is a valid directory
  std::string path = remus::worker::getExecutableLocation();
  boost::filesystem::path folder(path);

  bool is_dir = boost::filesystem::is_directory(folder);
  REMUS_ASSERT( is_dir );
}

void verify_find_file()
{
  // name = "", ext = "txt" = we don't search at all, we require a name
  {
  remus::common::FileHandle fh = remus::worker::findFile("", "txt");
  REMUS_ASSERT( (fh.size() == 0) );
  }

  {
  //try to find our selves
  std::string name("UnitTests_remus_worker_testing");
  std::string ext;
#ifdef _WIN32
  ext += ".exe";
#endif
  remus::common::FileHandle fh = remus::worker::findFile(name, ext);
  REMUS_ASSERT( (fh.size() != 0) );
  }
}

}

int UnitTestLocateFile(int, char *[])
{
  verify_executable_path();
  verify_find_file();
  return 0;
}
