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
# * Threads
# * filesystem
# * string algorithms
find_package(Boost 1.45.0 COMPONENTS thread filesystem system REQUIRED)

option(MeshServer_ENABLE_TESTING "Enable Testing" ON)
option(MeshServer_ENABLE_EXAMPLES "Enable Examples" ON)

#turn on ctest if we want testing
if (MeshServer_ENABLE_TESTING)
  enable_testing()
  include(CTest)
endif()


## Set the directory where the binaries will be stored
set( EXECUTABLE_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory where all executable will be stored"
)

## Set the directory where the libraries will be stored
set( LIBRARY_OUTPUT_PATH
  ${PROJECT_BINARY_DIR}/libs
  CACHE PATH
  "Directory where all the libraries will be stored"
)
mark_as_advanced(
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH)

#add this directory so we can include files like: <meshserver/internal/Test.h>
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

#setup the helper kwsys package, we just need system and process
#to launch external processes.
set(KWSYS_NAMESPACE sysTools)
set(KWSYS_USE_Process 1)
add_subdirectory(kwsys)


#add the actual code
add_subdirectory(meshserver)


#add the examples code
if(MeshServer_ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif()
