# Extends ExternalProject_Add(...) by adding a new option.
#  PROCESS_ENVIRONMENT <environment variables>
# When present the BUILD_COMMAND and CONFIGURE_COMMAND are executed as a
# sub-process (using execute_process()) so that the sepecified environment
# is passed on to the executed command (which does not happen by default).
# This will be deprecated once CMake starts supporting it.

include(ExternalProject)

string(REPLACE ")" "|PROCESS_ENVIRONMENT)"
  _ep_keywords_MKExternalProject_Add "${_ep_keywords_ExternalProject_Add}")

function (MKExternalProject_Add name)
  # process arguments are detect USE_ENVIRONMENT, BUILD_COMMAND and
  # CONFIGURE_COMMAND.

  # just create a temporary target so we can set target properties.
  add_custom_target(mk-${name})
  _ep_parse_arguments(MKExternalProject_Add mk-${name} _EP_ "${ARGN}")

  get_property(has_process_environment TARGET mk-${name}
    PROPERTY _EP_PROCESS_ENVIRONMENT SET)
  if (NOT has_process_environment)
    ExternalProject_Add(${name} ${ARGN})
    return()
  endif()

  set (new_argn)

  # check if we have a BUILD_COMMAND or CONFIGURE_COMMAND. 
  get_property(has_build_command TARGET mk-${name}
    PROPERTY _EP_BUILD_COMMAND SET)
  if(NOT has_build_command)
    # if no BUILD_COMMAND was specified, then the default build cmd is going to
    # be used, but then too we want to environment to be setup correctly. So we
    # obtain the default build command.
    _ep_get_build_command(mk-${name} BUILD cmd)
    if("${cmd}" MATCHES "^\\$\\(MAKE\\)")
      # GNU make recognizes the string "$(MAKE)" as recursive make, so
      # ensure that it appears directly in the makefile.
      string(REGEX REPLACE "^\\$\\(MAKE\\)" "${CMAKE_MAKE_PROGRAM} -j5" cmd "${cmd}")
    endif()

    set_property(TARGET mk-${name} PROPERTY _EP_BUILD_COMMAND "${cmd}")
    set (has_build_command 1)

    # when we customize the build command, the install command gets overridden
    # as well and we need to explicitly specify it too.
    get_property(has_install_command TARGET mk-${name} PROPERTY
    _EP_INSTALL_COMMAND SET)

    if (NOT has_install_command)
      _ep_get_build_command(mk-${name} INSTALL install_cmd)
      if (install_cmd)
        set (new_argn ${new_argn} INSTALL_COMMAND ${install_cmd})
      endif()
    endif()
  endif()

  get_property(has_configure_command TARGET mk-${name}
    PROPERTY _EP_CONFIGURE_COMMAND SET)

  if (has_configure_command)
    set(new_argn ${new_argn}
      CONFIGURE_COMMAND
      ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/mk-${name}-configure.cmake)
  endif()

  if (has_build_command)
    set(new_argn ${new_argn}
      BUILD_COMMAND 
      ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/mk-${name}-build.cmake)
  endif()

  # now strip PROCESS_ENVIRONMENT from argments.
  set (skip FALSE)
  foreach(arg IN LISTS ARGN)
    if (arg MATCHES "${_ep_keywords_MKExternalProject_Add}")
      if (arg MATCHES "^(PROCESS_ENVIRONMENT|BUILD_COMMAND|CONFIGURE_COMMAND)$")
        set (skip TRUE)
      else()
        set (skip FALSE)
      endif ()
    endif()
    if (NOT skip)
      list(APPEND new_argn ${arg})
    endif()
  endforeach()
  ExternalProject_Add(${name} ${new_argn})

  # configure the scripts after the call ExternalProject_Add() since that sets
  # up the directories correctly.
  get_target_property(process_environment mk-${name}
    _EP_PROCESS_ENVIRONMENT)
  _ep_replace_location_tags(${name} process_environment)

  if(WIN32)
    set(configure_file "win32_configure.cmake.in")
  else()
    set(configure_file "unix_configure.cmake.in")
  endif()
  if (has_configure_command)
    get_target_property(step_command mk-${name} _EP_CONFIGURE_COMMAND)
    _ep_replace_location_tags(${name} step_command)
    configure_file(${MeshServerSuperBuild_CMAKE_DIR}/${configure_file}
      ${CMAKE_CURRENT_BINARY_DIR}/mk-${name}-configure.cmake
      @ONLY
      )
  endif()

  if (has_build_command)
    get_target_property(step_command mk-${name} _EP_BUILD_COMMAND)
    _ep_replace_location_tags(${name} step_command)
    configure_file(${MeshServerSuperBuild_CMAKE_DIR}/${configure_file}
      ${CMAKE_CURRENT_BINARY_DIR}/mk-${name}-build.cmake
      @ONLY)
  endif()
endfunction()

function(MKExternalProject_AutoConf_Step name)
  ExternalProject_Add_Step(${name} ${name}-autoconf
    COMMAND autoreconf -fi <SOURCE_DIR>
    DEPENDEES update
    DEPENDERS configure
  )
endfunction()
