cmake_minimum_required (VERSION 3.1)

#setup policy rules for CMake 3.3 while we have a minimum required of 2.8.X
if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)#Honor visibility properties for all targets
endif()

#if the user/superbuild hasn't explicitly stated what c++ standard to use
#require C++11
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
  set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

project(Remus)

#------------------------------------------------------------------------------
#---------------------------- Remus Build Options -----------------------------
#------------------------------------------------------------------------------

option(Remus_NO_SYSTEM_BOOST "Allow boost to search for system installed boost" ON)
option(Remus_ENABLE_TESTING "Enable Testing" ON)
option(Remus_ENABLE_EXAMPLES "Enable Examples" ON)
option(BUILD_SHARED_LIBS "Build Remus using shared libraries" OFF)

#------------------------------------------------------------------------------
#---------------------------- Remus CMake Modules -----------------------------
#------------------------------------------------------------------------------

# include some cmake code that builds automatic header tests
# and can targets for unit tests
include(CMake/RemusMacros.cmake)

#make it so we can find our own FindModule Packages
list(APPEND CMAKE_MODULE_PATH "${Remus_SOURCE_DIR}/CMake")

#------------------------------------------------------------------------------
#----------------------------- Find Module Logic ------------------------------
#------------------------------------------------------------------------------

#ZeroMQ is used for all message passing between components.
find_package(ZeroMQ 3.0 REQUIRED)

#setup if we should use boost static libraries based on if we are
#building static or shared. We need to match boosts library type to ours so
#that we handle symbol visibility properly. On windows we really prefer
#static libraries so take that also into consideration.
if(NOT DEFINED Boost_USE_STATIC_LIBS)
  if(${BUILD_SHARED_LIBS})
    set(Boost_USE_STATIC_LIBS OFF)
  else()
    set(Boost_USE_STATIC_LIBS ON)
  endif()
endif()

#if boost is installed as a system library on a machine, we will never
#be able to set the superbuild to use the custom boost version. The solution
#is that when doing a system
set(Boost_NO_SYSTEM_PATHS ${Remus_NO_SYSTEM_BOOST})

# List of Boost features used:
# * Circular Buffers
# * C Types (uint16_t, int16_t, etc)
# * Date Time
# * Filesystem
# * Scoped Ptr
# * Shared Ptr
# * String Algorithms
# * Threads
# * Unordered Map
# * UUID Generation

if(WIN32)
  find_package(Boost 1.48.0
               COMPONENTS thread chrono filesystem system date_time REQUIRED)
  add_definitions(-DBOOST_ALL_NO_LIB)
  # Boost 1.70 and above need autolinking help from BoostConfig.cmake.
  add_definitions(-DBOOST_UUID_FORCE_AUTO_LINK)
else()
  find_package(Boost 1.45.0
               COMPONENTS thread filesystem system date_time REQUIRED)
endif()

#on unix/linux boost uses pthreads, so we need to find that, and link to it
#explicitly when using the boost thread library
if(UNIX)
  find_package(Threads REQUIRED)
endif()

#------------------------------------------------------------------------------
#-----------------------------Setup Build Flags  ------------------------------
#------------------------------------------------------------------------------
# include export header modules so that we can easily control symbol exporting
# Remus is setup by default not to export symbols unless explicitly stated.
include(GenerateExportHeader)

#-----------------------------------------------------------------------------
# Add supplemental compiler warnings, and GCC visibility support.
include(CMake/RemusCompilerExtras.cmake)

#------------------------------------------------------------------------------
#------------------------------- Setup Testing   ------------------------------
#------------------------------------------------------------------------------

#turn on ctest if we want testing
if (Remus_ENABLE_TESTING)
  include(CMake/RemusTestingMacros.cmake)
  enable_testing()
  include(CTest)
  mark_as_advanced(BUILD_TESTING)
endif()

#------------------------------------------------------------------------------
#--------------- Setup Library and Executable build locations -----------------
#------------------------------------------------------------------------------

## Set the directory where the binaries will be stored
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/bin
  CACHE PATH
  "Directory where all executable will be stored"
)

## Set the directory where the libraries will be stored
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
  CACHE PATH
  "Directory where all the libraries will be stored"
)
mark_as_advanced(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY
  CMAKE_LIBRARY_OUTPUT_DIRECTORY)

#------------------------------------------------------------------------------
#--------------------------- Setup Install Rules ------------------------------
#------------------------------------------------------------------------------

# enable @rpath in the install name for any shared library being built
# note: this is overwritten by INSTALL_NAME_DIR
set(CMAKE_MACOSX_RPATH 1)


#------------------------------------------------------------------------------
#----------------------------- Setup ThirdParty -------------------------------
#------------------------------------------------------------------------------

#setup the helper kwsys package, we just need system and process
#to launch external processes.
set(KWSYS_NAMESPACE RemusSysTools)
set(KWSYS_USE_Process 1)
set(KWSYS_USE_MD5 1)
set(KWSYS_INSTALL_LIB_DIR "lib")
set(KWSYS_INSTALL_BIN_DIR "bin")
set(KWSYS_BUILD_SHARED OFF) #we always want to build KWSYS statically
set(KWSYS_INSTALL_EXPORT_NAME Remus-targets)

add_subdirectory(thirdparty/kwsys)

if (BUILD_SHARED_LIBS)
  # On Mac OS X, set the directory included as part of the
  # installed library's path. We only do this to libraries that we plan
  # on installing
  set_target_properties(RemusSysTools PROPERTIES
                        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

  # When we are building shared we need to mark the static library as having
  # position independent code enabled so it can be embedded into dynamic libraries
  set_target_properties(RemusSysTools PROPERTIES
                        POSITION_INDEPENDENT_CODE True
                        )
endif()

#setup cJson as an object library
add_subdirectory(thirdparty/cJson)


#------------------------------------------------------------------------------
#---------------------------  Setup Visibility Flags  -------------------------
#------------------------------------------------------------------------------
# Set the visibility for all of remus. We do this after setting up the
# thirdparty code since it generates CMake warnings about visibility and static
# libraries.
# We prefer to only export symbols of a small set of user facing classes,
# rather than exporting all symbols. In theory we will try to not export
# symbols for any third party library
#
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)


#------------------------------------------------------------------------------
#------------------------------- Build Remus ----------------------------------
#------------------------------------------------------------------------------

#build the actual code
add_subdirectory(remus)

#build the examples
if(Remus_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

#------------------------------------------------------------------------------
#-------------------------- Install Support Files -----------------------------
#------------------------------------------------------------------------------
if (WIN32)
  set(cmakedir cmake)
else ()
  set(cmakedir lib/cmake)
endif ()
install(FILES ${CMAKE_MODULE_PATH}/RemusRegisterWorker.cmake
              ${CMAKE_MODULE_PATH}/FindZeroMQ.cmake
        DESTINATION ${cmakedir}/Remus
       )

remus_make_build_exports()
