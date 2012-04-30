# Build boost via its bootstrap script. The build tree cannot contain a space.
# This boost b2 build system yields errors with spaces in the name of the
# build dir.
#
if("${CMAKE_CURRENT_BINARY_DIR}" MATCHES " ")
  message(FATAL_ERROR "cannot use boost bootstrap with a space in the name of the build dir")
endif()

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

set(boost_cmds
  CONFIGURE_COMMAND ./bootstrap.sh --prefix=<INSTALL_DIR>
  BUILD_COMMAND ./b2 address-model=${am} ${boost_with_args}
  INSTALL_COMMAND ./b2 address-model=${am} ${boost_with_args}
    install
  )

add_external_project(boost
  ${boost_cmds}
  BUILD_IN_SOURCE 1
  )

#remove the copied boost include
ExternalProject_Add_Step(zeroMQ removeVersionedBoost
  COMMAND ${CMAKE_COMMAND} -E remove_directory <INSTALL_DIR>/include/boost-1_49/boost
  DEPENDEES install
  )

ExternalProject_Get_Property(boost install_dir)
set(BOOST_ROOT "${install_dir}" CACHE INTERNAL "")