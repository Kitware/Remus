#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# Utility to build a kit name from the current directory.
function(ms_get_kit_name kitvar)
  string(REPLACE "${Remus_SOURCE_DIR}/" "" dir_prefix ${CMAKE_CURRENT_SOURCE_DIR})
  string(REPLACE "/" "_" kit "${dir_prefix}")
  set(${kitvar} "${kit}" PARENT_SCOPE)
  # Optional second argument to get dir_prefix.
  if (${ARGC} GREATER 1)
    set(${ARGV1} "${dir_prefix}" PARENT_SCOPE)
  endif (${ARGC} GREATER 1)
endfunction(ms_get_kit_name)


# Builds a source file and an executable that does nothing other than
# compile the given header files.
function(ms_add_header_test name dir_prefix)
  if(Remus_ENABLE_TESTING)
    set(hfiles ${ARGN})
    set(suffix ".cxx")
    set(cxxfiles)
    foreach (header ${ARGN})
      string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}" "" header "${header}")
      get_filename_component(headername ${header} NAME_WE)
      set(src ${CMAKE_CURRENT_BINARY_DIR}/TestBuild_${name}_${headername}${suffix})
      configure_file(${Remus_SOURCE_DIR}/CMake/TestBuild.cxx.in ${src} @ONLY)
      set(cxxfiles ${cxxfiles} ${src})
    endforeach (header)
    include_directories(${sysTools_BINARY_DIR})
    #include the build directory for the export header
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_library(TestBuild_${name} ${cxxfiles} ${hfiles})
    target_link_libraries(TestBuild_${name} sysTools ${ZeroMQ_LIBRARIES})
    set_source_files_properties(${hfiles}
      PROPERTIES HEADER_FILE_ONLY TRUE
      )
  endif()
endfunction(ms_add_header_test)

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE. Also makes sure we install the headers
# into the include folder
function(remus_public_headers)
  ms_get_kit_name(name dir_prefix)
  ms_add_header_test("${name}" "${dir_prefix}" ${ARGN})
  install (FILES ${ARGN} DESTINATION include/${dir_prefix})
endfunction(remus_public_headers)

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE.
function(remus_private_headers)
  ms_get_kit_name(name dir_prefix)
  ms_add_header_test("${name}" "${dir_prefix}" ${ARGN})
endfunction(remus_private_headers)

# Declare a library as needed to be installed
function(remus_install_library target)
  install(TARGETS ${target} DESTINATION lib EXPORT Remus-targets)
endfunction(remus_install_library)


#setup include directories as target properties
function(remus_set_includes target)
  #attach the current build and source directory
  set(full_includes ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${ARGN}
                    )
  #include everything
  include_directories(${full_includes})

  #set up a property on the passed in target
  set_property(TARGET ${target} PROPERTY SAVED_INCLUDE_DIRS ${full_includes})
endfunction(remus_set_includes)

#read the include directory proptery for a target and create a variable
#in the callers scope with the name past names as the variable includes_var_name
function(remus_get_includes target includes_var_name)
  get_property(saved_includes TARGET ${target} PROPERTY SAVED_INCLUDE_DIRS)
  set(${includes_var_name} ${saved_includes} PARENT_SCOPE)
endfunction(remus_get_includes)


#generate an export header and create an install target for it
function(remus_export_header target file)
  ms_get_kit_name(name dir_prefix)
  generate_export_header(${target} EXPORT_FILE_NAME ${file})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}  DESTINATION include/${dir_prefix})
endfunction(remus_export_header)


# Declare unit tests Usage:
#
# remus_unit_tests(
#   SOURCES <source_list>
#   LIBRARIES <dependent_library_list>
#   )
function(remus_unit_tests)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES LIBRARIES)
  cmake_parse_arguments(Remus_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  if (Remus_ENABLE_TESTING)
    ms_get_kit_name(kit)
    #we use UnitTests_ so that it is an unique key to exclude from coverage
    set(test_prog UnitTests_${kit})
    create_test_sourcelist(TestSources ${test_prog}.cxx ${Remus_ut_SOURCES})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_executable(${test_prog} ${TestSources})
    target_link_libraries(${test_prog} ${Remus_ut_LIBRARIES})
    foreach (test ${Remus_ut_SOURCES})
      get_filename_component(tname ${test} NAME_WE)
      add_test(NAME ${tname}
        COMMAND ${test_prog} ${tname}
        )
    endforeach(test)
  endif (Remus_ENABLE_TESTING)
endfunction(remus_unit_tests)

# Declare unit test executables that are needed by other unit_tests
# Usage:
#
# remus_unit_test_executable(
#   EXEC_NAME <name>
#   SOURCES <source_list>
#   LIBRARIES <dependent_library_list>
#   )
function(remus_unit_test_executable)
  set(options)
  set(oneValueArgs EXEC_NAME)
  set(multiValueArgs SOURCES LIBRARIES)
  cmake_parse_arguments(Remus_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  if (Remus_ENABLE_TESTING)
    set(test_prog ${Remus_ut_EXEC_NAME})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    add_executable(${test_prog} ${Remus_ut_SOURCES})
    target_link_libraries(${test_prog} ${Remus_ut_LIBRARIES})

  endif (Remus_ENABLE_TESTING)
endfunction(remus_unit_test_executable)
