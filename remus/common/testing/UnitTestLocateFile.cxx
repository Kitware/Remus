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

#include <remus/common/LocateFile.h>
#include <remus/testing/Testing.h>

#include <string>

REMUS_THIRDPARTY_PRE_INCLUDE
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace {

using namespace remus::common;

void verify_executable_path()
{
  //Test that the path coming back is a valid directory
  std::string path = remus::common::getExecutableLocation();
  boost::filesystem::path folder(path);

  bool is_dir = boost::filesystem::is_directory(folder);
  REMUS_ASSERT( is_dir );
}

void verify_find_file()
{
  // name = "", ext = "txt" = we don't search at all, we require a name
  {
  remus::common::FileHandle fh = remus::common::findFile("", "txt");
  REMUS_ASSERT( (fh.size() == 0) );
  }

  {
  //try to find our selves
  std::string name("UnitTests_remus_common_testing");
  std::string ext;
#ifdef _WIN32
  ext += ".exe";
#endif
  remus::common::FileHandle fh = remus::common::findFile(name, ext);
  REMUS_ASSERT( (fh.size() != 0) );
  }
}

void verify_relative_locations_to_search()
{
  std::string exec_path = remus::common::getExecutableLocation();
  std::vector<std::string> locations = remus::common::relativeLocationsToSearch();

  typedef std::vector<std::string>::const_iterator It;
  bool is_dir = false;
  for(It i = locations.begin(); i != locations.end() && !is_dir;++i)
    {
      //at least 1 of these paths should be valid
    std::stringstream buffer;
    buffer << exec_path << "/" << *i;

    boost::filesystem::path folder(buffer.str());
    is_dir = boost::filesystem::is_directory(folder);
    std::cout << "folder: " << folder << " is valid : " << is_dir << std::endl;
    }
  REMUS_ASSERT( (is_dir==true) );



}

}

int UnitTestLocateFile(int, char *[])
{
  verify_executable_path();
  verify_find_file();

  verify_relative_locations_to_search();
  return 0;
}
