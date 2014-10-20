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
      set_tests_properties(${tname} PROPERTIES TIMEOUT 120)
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
#   IS_FILE_BASED
#   )

# IS_FILE_BASED will set the requirements to be file based, and specify
# the file format to be USER, and will point back to the file we are generating
# as the requirements file

function(remus_register_unit_test_worker)

  #enable only the new parser for this function. Policies are scoped to the
  #function so we don't have to worry about this affecting the calling project
  if(POLICY CMP0053)
    cmake_policy(SET CMP0053 NEW)
  endif()

  set(options IS_FILE_BASED)
  set(oneValueArgs EXEC_NAME INPUT_TYPE OUTPUT_TYPE CONFIG_DIR FILE_EXT)
  set(multiValueArgs)
  cmake_parse_arguments(R
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  #set up variables that the config file is looking for
  set(input_type "${R_INPUT_TYPE}")
  set(output_type "${R_OUTPUT_TYPE}")
  set(worker_name "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${R_EXEC_NAME}")
  set(reqs_file_name ${R_EXEC_NAME}.${R_FILE_EXT})

  set(R_ABS_FILE_NAME ${R_CONFIG_DIR}/${R_EXEC_NAME}.${R_FILE_EXT})
  if(R_IS_FILE_BASED)
    set(rw_file_content
    "
    {
    \"ExecutableName\": \"@worker_name@\",
    \"InputType\": \"@input_type@\",
    \"OutputType\": \"@output_type@\",
    \"File\" : \"@reqs_file_name@\",
    \"FileFormat\" : \"USER\"
    }
    "
    )
    string(CONFIGURE "${rw_file_content}" file_contents @ONLY)
    file(WRITE "${R_ABS_FILE_NAME}" "${file_contents}")
  else()
    set(rw_file_content
    "
    {
    \"ExecutableName\": \"@worker_name@\",
    \"InputType\": \"@input_type@\",
    \"OutputType\": \"@output_type@\"
    }
    "
    )
    string(CONFIGURE "${rw_file_content}" file_contents @ONLY)
    file(WRITE "${R_ABS_FILE_NAME}" "${file_contents}")
  endif()
endfunction()

# Declare integration tests Usage:
#
# remus_integration_tests(
#   SOURCES <test_source_list>
#   EXTRA_SOURCES <helper_source_files>
#   LIBRARIES <dependent_library_list>
#   )
# unlike unit tests, integration tests link to all the remus libraries
# automatically, as they are verifying the components work properly together
function(remus_integration_tests)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES EXTRA_SOURCES LIBRARIES)
  cmake_parse_arguments(Remus_it
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN}
    )

  if (Remus_ENABLE_TESTING)
    ms_get_kit_name(kit)
    #we use IntegrationTests_ so that it is an unique key to exclude from coverage
    set(test_prog IntegrationTests_${kit})
    create_test_sourcelist(TestSources ${test_prog}.cxx ${Remus_it_SOURCES})

    add_executable(${test_prog} ${TestSources} ${Remus_it_EXTRA_SOURCES})
    target_link_libraries(${test_prog}
                          LINK_PRIVATE
                          RemusClient
                          RemusWorker
                          RemusServer
                          ${Remus_it_LIBRARIES})
    target_include_directories(${test_prog} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    foreach (test ${Remus_it_SOURCES})
      get_filename_component(tname ${test} NAME_WE)
      add_test(NAME ${tname} COMMAND ${test_prog} ${tname})
      #we give integration tests a longer timout peroid, since they
      #have 'real' work
      set_tests_properties(${tname} PROPERTIES TIMEOUT 240)
    endforeach(test)
  endif (Remus_ENABLE_TESTING)
endfunction(remus_integration_tests)
