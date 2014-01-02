if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(am 64)
else()
  set(am 32)
endif()

set(boost_with_args
  --with-date_time
  --with-filesystem
  --with-system
  --with-thread
  )

#since we don't specify a prefix for the superbuild,
#we can determine where the buld directory will be. This
#is needed as we need to wrap the build directory in quotes to
#and escape spaces in the path for boost to properly build.
string(REPLACE " " "\\ " boost_build_dir ${CMAKE_CURRENT_BINARY_DIR}/boost/src/boost)

#we don't convert the install path on windows, as for the install step it can't
#handle escaping the spaces. I have no clue why boost is
#such a mess on support for directories with spaces in the name

if(MSVC)
  if(MSVC11)
    set(msvc_version "msvc-11.0")
  elseif(MSVC10)
    set(msvc_version "msvc-10.0")
  elseif(MSVC90)
    set(msvc_version "msvc-9.0")
  elseif(MSVC80)
    set(msvc_version "msvc-8.0")
  endif()

  set(boost_cmds
    CONFIGURE_COMMAND bootstrap.bat
    BUILD_COMMAND b2 --build-dir=${boost_build_dir} toolset=${msvc_version} address-model=${am} ${boost_with_args}
    INSTALL_COMMAND b2 toolset=${msvc_version} address-model=${am} ${boost_with_args} --prefix=${install_location} install
    )
else()
  set(boost_cmds
    CONFIGURE_COMMAND bootstrap.bat
    BUILD_COMMAND b2 --build-dir=${boost_build_dir} address-model=${am} ${boost_with_args}
    INSTALL_COMMAND b2 address-model=${am} ${boost_with_args} --prefix=${install_location} install
    )
endif()

  add_external_project(boost
    ${boost_cmds}
    BUILD_IN_SOURCE 1
    )

  #install header files
  ExternalProject_Add_Step(boost fixBoostInstallStage1
    COMMAND ${CMAKE_COMMAND} -E copy_directory <INSTALL_DIR>/include/boost-1_49/boost <INSTALL_DIR>/include/boost
    DEPENDEES install
    )
  ExternalProject_Add_Step(boost fixBoostInstallStage2
    COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/boost-1_49
    DEPENDEES fixBoostInstallStage1
    )

ExternalProject_Get_Property(boost install_dir)
add_project_property(boost BOOST_INCLUDEDIR "${install_dir}/include/boost")
add_project_property(boost BOOST_LIBRARYDIR "${install_dir}/lib")
