cmake_minimum_required(VERSION 2.8)
project(RemusSuperBuild)
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
  "${CMAKE_CURRENT_SOURCE_DIR}/Projects/${platform}"
  "${CMAKE_CURRENT_SOURCE_DIR}/Projects/"
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  ${CMAKE_MODULE_PATH}
  )

set (RemusSuperBuild_CMAKE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include (RemusModules)
include (${CMAKE_CURRENT_SOURCE_DIR}/versions.cmake)

if(NOT REMUS_INSTALL_DIR)
  set(REMUS_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE FILEPATH "Location to build Remus.")
endif()

set (install_location "${REMUS_INSTALL_DIR}")
set (download_location "${CMAKE_CURRENT_BINARY_DIR}/downloads")
set (prefix_path "${install_location}")

add_project(zeroMQ SYSTEM REQUIRED)
add_project(boost SYSTEM REQUIRED)
add_project(remus REQUIRED)
