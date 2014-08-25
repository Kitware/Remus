#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)

  include(CheckCXXCompilerFlag)

  #Add option for enabling gcov coverage -- GCC only
  if(CMAKE_COMPILER_IS_GNUCXX)
    option(Remus_ENABLE_COVERAGE "Build with gcov support." OFF)
    mark_as_advanced(Remus_ENABLE_COVERAGE)
  endif(CMAKE_COMPILER_IS_GNUCXX)

  #Only enabling coverage if we're GCC
  if(Remus_ENABLE_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    #We're setting the CXX flags and C flags beacuse they're propogated down
    #independent of build type.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fno-elide-constructors")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

    #setup custom exclude for Remus, we do this by creating a
    #CTestCutom.cmake file in the build tree
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
      #try using "-p" to preserve full paths to handle the fact that we have
      #to files named Job.h/Job.cxx
      "
      set(COVERAGE_EXTRA_FLAGS \"-l -p\")
      set(CTEST_CUSTOM_COVERAGE_EXCLUDE
        \"thirdparty\"
        \"zmq.hpp\"
        \"UnitTest\"
        \"TestBuild_remus_\"
        \"/remus/testing\"
        )
      ")


  endif()

  # Standard warning flags we should always have
  set(CMAKE_CXX_FLAGS_WARN " -Wall")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS_WARN}")
  set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_WARN}")

  # Addtional warnings for GCC and clang
  set(CMAKE_CXX_FLAGS_WARN_EXTRA "-Wno-long-long -Wcast-align -Wchar-subscripts -Wextra -Wpointer-arith -Wformat -Wformat-security -Wshadow -Wunused-parameter -fno-common")

  # Set up the debug CXX_FLAGS for extra warnings
  option(Remus_EXTRA_COMPILER_WARNINGS "Add compiler flags to do stricter checking when building debug." ON)
  if(Remus_EXTRA_COMPILER_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
      "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_CXX_FLAGS_WARN_EXTRA}")
    set(CMAKE_CXX_FLAGS_DEBUG
      "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS_WARN_EXTRA}")
  endif()
endif()

#setup windows exception handling so we can compile properly with boost enabled
if(WIN32 AND MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()
