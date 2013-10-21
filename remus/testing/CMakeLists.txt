
set(headers
  Testing.h
  )

#install the headers
remus_public_headers(${headers})

if(Remus_ENABLE_TESTING)
  #if testing is enabled we add a unit test for
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND}
      "-DRemus_SOURCE_DIR=${Remus_SOURCE_DIR}" -P "${Remus_SOURCE_DIR}/CMake/CheckCopyright.cmake"
    )
endif()
