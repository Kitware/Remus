

#for windows the system is far more complex as we have to
#adding in correct solution files and than call the right one

#the first step is to create an update step that moves the solution files
#to the zeroMQ directory

function(build_zeroMQ_command command)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(zeroMQ_configuration "Release|x64")
  else()
    set(zeroMQ_configuration "Release|Win32")
  endif()

  set(${command} "/build ${zeroMQ_configuration} /project libzmq" PARENT_SCOPE)  
endfunction(build_zeroMQ_command)

if(MSVC)
  if(MSVC10)
    set(zeroMQ_sln_name "zeroMQ2010.sln")
  elseif(MSVC09)
    set(zeroMQ_sln_name "zeroMQ2008.sln")
  else()
    message(FATAL_ERROR "We only support 2008 and 2010")
  endif()
  set(zeroMQ_configure_sln ${CMAKE_CURRENT_SOURCE_DIR}/CMake/win32/${zeroMQ_sln_name})
  
  #get the arguments for devenv
  build_zeroMQ_command(buildCommand)

  #add in a configure step that properly copies the solution files to zeroMQ source tree
  #don't use add_external_project that is a unix helper cuurently, so copy the prefix, downloaddir
  #install dir and git url
  ExternalProject_Add(zeroMQ
    PREFIX zeroMQ
    DOWNLOAD_DIR ${download_location}
    INSTALL_DIR ${install_location}
    # add url/mdf/git-repo etc. specified in versions.cmake
    ${zeroMQ_revision}
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ${zeroMQ_configure_sln}  <SOURCE_DIR>/builds/msvc/${zeroMQ_sln_name}
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} <SOURCE_DIR>/builds/msvc/${zeroMQ_sln_name} ${buildCommand} 
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo
    )

  message(STATUS "<SOURCE_DIR>/builds/msvc/${zeroMQ_sln_name}" )

  #add in a custom post install command that copy the zeroMQ dll and headers to the install directory
  ExternalProject_Add_Step(zeroMQ installDll
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libzmq.dll <INSTALL_DIR>/lib/libzmq.dll
    DEPENDEES install
    )

  ExternalProject_Add_Step(zeroMQ installLib
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/builds/msvc/Release/libzmq.lib <INSTALL_DIR>/lib/libzmq.lib
    DEPENDEES install
    )

  ExternalProject_Add_Step(zeroMQ installHeaders
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/* <INSTALL_DIR>/include/
    DEPENDEES install
    )

elseif()
  add_external_project(zeroMQ
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>)
endif(MSVC)


