
add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(worker)


#include Boost
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ZeroMQ_INCLUDE_DIR})

set(headers
  Job.h
  JobResult.h
  JobRequest.h
  JobStatus.h
  version.h
  )

remus_public_headers(${headers})

#enable tests
if(Remus_ENABLE_TESTING)
  #add in the test for copy right statements
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND} "-DRemus_SOURCE_DIR=${Remus_SOURCE_DIR}" -P "${Remus_SOURCE_DIR}/CMake/CheckCopyright.cmake"
    )
endif()


#setup the exports for the library when used from an installed location
install(EXPORT Remus-targets DESTINATION lib)
