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

  const bool is_dir = boost::filesystem::is_directory(folder);
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

void verify_temp_dir_is_valid()
{
  //Test that the path coming back is a valid directory
  std::string path = remus::common::getTempLocation();
  boost::filesystem::path folder(path);

  const bool is_dir = boost::filesystem::is_directory(folder);
  REMUS_ASSERT( is_dir );
}

void verify_temp_file_is_writable()
{
  const std::string name = "tempfile";
  const std::string ext = ".log";

  remus::common::FileHandle fh = remus::common::makeTempFileHandle(name,ext);

  //now try to write to said file
  std::fstream f(fh.data(), std::ios::binary | std::ios::out );
  REMUS_ASSERT( f.is_open() );
  REMUS_ASSERT( f.good() );

  f << "Hello temp file" << std::endl;
  REMUS_ASSERT( f.good() );

  //now that we have written to file verify it is on disk
  const bool is_file = boost::filesystem::is_regular_file( fh.path() );
  REMUS_ASSERT( is_file );

  //remove said file
  const bool is_removed = boost::filesystem::remove( fh.path() );
  REMUS_ASSERT( is_removed );
}

void verify_temp_file_handles_duplicates()
{
  const std::string name = "tempfile";
  const std::string ext = "log";

  remus::common::FileHandle fh1 = remus::common::makeTempFileHandle(name,ext);

  //now open fh1 so in-actuality the file exists.
  std::fstream f(fh1.data(), std::ios::binary | std::ios::out );


  remus::common::FileHandle fh2 = remus::common::makeTempFileHandle(name,ext);
  REMUS_ASSERT( (fh2.path() != fh1.path()) );


  //delete the file now
  const bool is_removed = boost::filesystem::remove( fh1.path() );
  REMUS_ASSERT( is_removed );
}

int UnitTestLocateFile(int, char *[])
{
  verify_executable_path();
  verify_find_file();

  verify_relative_locations_to_search();

  verify_temp_dir_is_valid();
  verify_temp_file_is_writable();
  verify_temp_file_handles_duplicates();
  return 0;
}
