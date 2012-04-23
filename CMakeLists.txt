cmake_minimum_required(VERSION 2.8)
project(MeshServerSuperBuild)
set (platform)
if (UNIX)
  set (platform "unix")
elseif (APPLE)
  set (platform "apple")
elseif (WIN32)
  set (platform "win32")
endif()

# Setup CMAKE_MODULE_PATH so that platform specific configurations are processed
# before the generic ones.
set (CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/${platform}"
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  ${CMAKE_MODULE_PATH}
  )

set (MeshServerSuperBuild_CMAKE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include (MeshServerModules)
include (${CMAKE_CURRENT_SOURCE_DIR}/versions.cmake)

set (install_location "${CMAKE_CURRENT_BINARY_DIR}/install")
set (download_location "${CMAKE_CURRENT_BINARY_DIR}/downloads")

set (ldflags "$ENV{LDFLAGS} -L${install_location}/lib")
set (cppflags "$ENV{CPPFLAGS} -I${install_location}/include -fPIC")
set (cxxflags "$ENV{CXXFLAGS} -I${install_location}/include -fPIC")
set (cflags "$ENV{CFLAGS} -I${install_location}/include")
set (ld_library_path "${install_location}/lib:$ENV{LD_LIBRARY_PATH}")
set (prefix_path "${install_location}")

string (STRIP "${ldflags}" ldflags)
string (STRIP "${cppflags}" cppflags)
string (STRIP "${cxxflags}" cxxflags)
string (STRIP "${cflags}"  cflags)

add_project(zeroMQ REQUIRED)
add_project(boost REQUIRED)
add_project(meshserver REQUIRED)
