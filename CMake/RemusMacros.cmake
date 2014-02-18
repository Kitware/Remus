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

  #include the build directory for the export header
  #include ZeroMQ and boost for the testing framework
  add_library(TestBuild_${name} ${cxxfiles} ${hfiles})
  target_include_directories(TestBuild_${name}
        PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE ${ZeroMQ_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}}
        )

  set_source_files_properties(${hfiles}
    PROPERTIES HEADER_FILE_ONLY TRUE
    )
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

#generate an export header and create an install target for it
function(remus_export_header target file)
  ms_get_kit_name(name dir_prefix)
  generate_export_header(${target} EXPORT_FILE_NAME ${file})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}  DESTINATION include/${dir_prefix})
endfunction(remus_export_header)


# Declare unit tests Usage:
#
# remus_unit_tests(
#   SOURCES <test_source_list>
#   EXTRA_SOURCES <helper_source_files>
#   LIBRARIES <dependent_library_list>
#   )
function(remus_unit_tests)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES EXTRA_SOURCES LIBRARIES)
  cmake_parse_arguments(Remus_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  if (Remus_ENABLE_TESTING)
    ms_get_kit_name(kit)
    #we use UnitTests_ so that it is an unique key to exclude from coverage
    set(test_prog UnitTests_${kit})
    create_test_sourcelist(TestSources ${test_prog}.cxx ${Remus_ut_SOURCES})

    add_executable(${test_prog} ${TestSources} ${Remus_ut_EXTRA_SOURCES})
    target_link_libraries(${test_prog} LINK_PRIVATE ${Remus_ut_LIBRARIES})
    target_include_directories(${test_prog} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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
    add_executable(${test_prog} ${Remus_ut_SOURCES})
    target_include_directories(${test_prog} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${test_prog} LINK_PRIVATE ${Remus_ut_LIBRARIES})

  endif (Remus_ENABLE_TESTING)
endfunction(remus_unit_test_executable)

# Declare unit test remus worker that is needed by other unit_tests
# Usage:
#
# remus_register_unit_test_worker(
#   EXEC_NAME <name>
#   INPUT_TYPE <MeshInputType>
#   OUTPUT_TYPE <MeshOuputType>
#   CONFIG_DIR <LocationToConfigureAt>
#   FILE_EXT  <FileExtOfWorker>
#   )

function(remus_register_unit_test_worker)
  set(options)
  set(oneValueArgs EXEC_NAME INPUT_TYPE OUTPUT_TYPE CONFIG_DIR FILE_EXT)
  set(multiValueArgs)
  cmake_parse_arguments(Remus_ut
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  #set up variables that the config file is looking for
  set(InputMeshFileType ${Remus_ut_INPUT_TYPE})
  set(OutputMeshType ${Remus_ut_OUTPUT_TYPE})
  set(workerExecutableName "${EXECUTABLE_OUTPUT_PATH}/${Remus_ut_EXEC_NAME}")

  configure_file(
          ${Remus_SOURCE_DIR}/CMake/RemusWorker.rw.in
          ${Remus_ut_CONFIG_DIR}/${Remus_ut_EXEC_NAME}.${Remus_ut_FILE_EXT}
          @ONLY)
endfunction()
