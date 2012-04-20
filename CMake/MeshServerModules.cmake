include(MeshServerExternalProject)
include(CMakeParseArguments)

# Function to provide an option only if a set of other variables are ON.
# Example invocation:
#
#  dependent_option(USE_FOO "Use Foo" ON "USE_BAR;USE_ZOT" OFF)
#
# If both USE_BAR and USE_ZOT are true, this provides an option called
# USE_FOO that defaults to ON.  Otherwise, it sets USE_FOO to OFF.  If
# the status of USE_BAR or USE_ZOT ever changes, any value for the
# USE_FOO option is saved so that when the option is re-enabled it
# retains its old value.
#
function(dependent_option option doc default depends force)
  if (${option}_ISSET MATCHES "^${option}_ISSET$")
    set(${option}_AVAILABLE 1)
    foreach (d ${depends})
      if (NOT ${d})
        set(${option}_AVAILABLE 0)
      endif()
    endforeach()

    if (${option}_AVAILABLE)
      option(${option} "${doc}" "${default}")
      set(${option} "${${option}}" CACHE BOOL "${doc}" FORCE)
    else ()
      if(NOT ${option} MATCHES "^${option}$")
        set(${option} "${${option}}" CACHE INTERNAL "${doc}")
      endif ()
      set (${option} ${force})
    endif()
  else()
    set(${option} "${${option}_ISSET}")
  endif() 
endfunction()

function(add_project name)
  cmake_parse_arguments(arg "REQUIRED;DEFAULT_OFF" "" "DEPENDS" ${ARGN})
  string(TOUPPER ${name} UNAME)
  if (arg_REQUIRED)
    set(ENABLE_${UNAME} ON CACHE INTERNAL "Project '${name}'" FORCE)
  else()
    if (NOT arg_DEFAULT_OFF)
      option(ENABLE_${UNAME} "Enable sub-project '${name}'" ON)
    else()
      option(ENABLE_${UNAME} "Enable sub-project '${name}'" OFF)
    endif()
    mark_as_advanced(ENABLE_${UNAME})
  endif()

  if (ENABLE_${UNAME})
    # verify each of the required dependencies is enabled.
    foreach (required_dependency IN LISTS arg_DEPENDS)
      string (TOUPPER ${required_dependency} UREQUIRED_DEPENDENCY)
      if (NOT ENABLE_${UREQUIRED_DEPENDENCY})
        message(WARNING
          "${name} needs ${required_dependency} which is OFF, however."
          "Ignoring ENABLE_${UNAME}.")
        set (ENABLE_${UNAME} OFF)
        break()
      endif()
    endforeach()
  endif()

  if (ENABLE_${UNAME})
    # check for platform specific file. It none exists, try the default file.
    include("${name}" RESULT_VARIABLE rv)
    message(STATUS "Using configuration ${rv}")
  else ()
    # add dummy target to dependencies work even with subproject is disabled.
    add_custom_target(${name})
    SET (NO_${UNAME} TRUE)
  endif()
endfunction()

function(add_external_project name)
  #remove autoconf as it is used to add a custom external project step
  cmake_parse_arguments(arg "USE_AUTOCONF" "" "" ${ARGN})
  list(REMOVE_ITEM ARGN "USE_AUTOCONF")

  set (cmake_params)
  foreach (flag CMAKE_BUILD_TYPE
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_MINSIZEREL
                CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_RELWITHDEBINFO
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_MINSIZEREL
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (flag)
      list (APPEND cmake_params -D${flag}:STRING=${${flag}})
    endif()
  endforeach()

  MKExternalProject_Add(${name} ${ARGN}
    PREFIX ${name}
    DOWNLOAD_DIR ${download_location}
    INSTALL_DIR ${install_location}

    # add url/mdf/git-repo etc. specified in versions.cmake
    ${${name}_revision}

    PROCESS_ENVIRONMENT
      LDFLAGS "${ldflags}"
      CPPFLAGS "${cppflags}"
      CXXFLAGS "${cxxflags}"
      CFLAGS "${cflags}"
      LD_LIBRARY_PATH "${ld_library_path}"
      CMAKE_PREFIX_PATH "${prefix_path}"
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX:PATH=${prefix_path}
      -DCMAKE_PREFIX_PATH:PATH=${prefix_path} 
      -DCMAKE_C_FLAGS:STRING=${cflags}
      -DCMAKE_CXX_FLAGS:STRING=${cppflags}
      -DCMAKE_SHARED_LINKER_FLAGS:STRING=${ldflags}
      ${cmake_params}
    )
  if(arg_USE_AUTOCONF)
    MKExternalProject_AutoConf_Step(${name})
  endif()
endfunction()

function(add_revision name)
  set(${name}_revision "${ARGN}" CACHE INTERNAL
      "Revision for ${name}")
endfunction()
