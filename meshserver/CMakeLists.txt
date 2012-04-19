
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(worker)


#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
  Client.h
  JobMessage.h
  JobResponse.h
  )

meshserver_public_headers(${headers})

#enable tests
if(MeshServer_ENABLE_TESTING) 
  #add in the test for copy right statements
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND} "-DMeshServer_SOURCE_DIR=${MeshServer_SOURCE_DIR}" -P "${MeshServer_SOURCE_DIR}/CMake/CheckCopyright.cmake"
    )
endif()
