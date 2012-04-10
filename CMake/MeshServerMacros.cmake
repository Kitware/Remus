#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# Utility to build a kit name from the current directory.
function(ms_get_kit_name kitvar)
  string(REPLACE "${MeshServer_SOURCE_DIR}/" "" dir_prefix ${CMAKE_CURRENT_SOURCE_DIR})
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
    configure_file(${MeshServer_SOURCE_DIR}/CMake/TestBuild.cxx.in ${src} @ONLY)
    set(cxxfiles ${cxxfiles} ${src})
  endforeach (header)
  include_directories(${sysTools_BINARY_DIR})
  add_library(TestBuild_${name} ${cxxfiles} ${hfiles})
  target_link_libraries(TestBuild_${name} sysTools)
  set_source_files_properties(${hfiles}
    PROPERTIES HEADER_FILE_ONLY TRUE
    )
endfunction(ms_add_header_test)

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE. Also makes sure we install the headers
# into the include folder
function(meshserver_public_headers)
  ms_get_kit_name(name dir_prefix)
  ms_add_header_test("${name}" "${dir_prefix}" ${ARGN})
  install (FILES ${ARGN} DESTINATION include/meshserver)
endfunction(meshserver_public_headers)

# Declare a list of header files.  Will make sure the header files get
# compiled and show up in an IDE.
function(meshserver_private_headers)
  ms_get_kit_name(name dir_prefix)
  ms_add_header_test("${name}" "${dir_prefix}" ${ARGN})
endfunction(meshserver_private_headers)
