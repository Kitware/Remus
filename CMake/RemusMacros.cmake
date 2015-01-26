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

    #include the build directory for the export header
    #include ZeroMQ and boost for the testing framework
    add_library(TestBuild_${name} ${cxxfiles} ${hfiles})
    target_include_directories(TestBuild_${name}
          PUBLIC  ${CMAKE_CURRENT_BINARY_DIR}
          PRIVATE ${ZeroMQ_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
          )
    if(MSVC)
      target_compile_definitions(TestBuild_${name} PRIVATE
       _SCL_SECURE_NO_WARNINGS
       _CRT_SECURE_NO_WARNINGS
      )
    endif()

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
  ms_add_header_test("${name}_private" "${dir_prefix}" ${ARGN})
endfunction(remus_private_headers)

# Declare a library as needed to be installed
function(remus_install_library target)
  install(TARGETS ${target}
          EXPORT Remus-targets
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          )

  # On Mac OS X, set the directory included as part of the
  # installed library's path. We only do this to libraries that we plan
  # on installing
  if (BUILD_SHARED_LIBS)
    set_target_properties(${target} PROPERTIES
                          INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  endif()

endfunction(remus_install_library)

function(remus_install_third_party_library target)
  install(TARGETS ${target} DESTINATION lib)
endfunction(remus_install_third_party_library)

#generate an export header and create an install target for it
function(remus_export_header target file)
  ms_get_kit_name(name dir_prefix)
  generate_export_header(${target} EXPORT_FILE_NAME ${file})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}  DESTINATION include/${dir_prefix})
endfunction(remus_export_header)
