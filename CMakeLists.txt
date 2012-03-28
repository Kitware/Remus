cmake_minimum_required (VERSION 2.8)
project(MeshServer)

# include some cmake code that builds automatic header tests
# and can targets for unit tests
include(CMake/MeshServerMacros.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MeshServer_SOURCE_DIR}/CMake)

#ZeroMQ is used for all message passing between components.
find_package(ZeroMQ REQUIRED)

# List of Boost features used:
# * UUID Generation
# * Shared Ptr
# * Scoped Ptr
find_package(Boost 1.45.0 REQUIRED)

add_subdirectory(meshserver)
