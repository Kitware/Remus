

#for windows the system is far more complex as we have to
#adding in correct solution files and than call the right one

#the first step is to create an update step that moves the solution files
#to the zeroMQ directory

function(build_zeroMQ_command command solution)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(${command} "${CMAKE_MAKE_PROGRAM}" <SOURCE_DIR>/builds/msvc/${solution} /build "Release|x64" /project libzmq PARENT_SCOPE)
  else()
    set(${command} "${CMAKE_MAKE_PROGRAM}" <SOURCE_DIR>/builds/msvc/${solution} /build "Release|Win32" /project libzmq PARENT_SCOPE)
  endif()
endfunction(build_zeroMQ_command)

function(zeroMQ_libDir path)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(${path} <SOURCE_DIR>/builds/msvc/x64/Release/libzmq.lib PARENT_SCOPE)
  else()
    set(${path} <SOURCE_DIR>/builds/msvc/Win32/Release/libzmq.lib PARENT_SCOPE)
  endif()
endfunction(zeroMQ_libDir)

if(MSVC)
  
  set(zeroMQ_sln_name "zeroMQ.sln")
  set(zeroMQ_configure_sln ${CMAKE_CURRENT_SOURCE_DIR}/CMake/win32/${zeroMQ_sln_name})
  set(zeroMQ_vcproj ${CMAKE_CURRENT_SOURCE_DIR}/CMake/win32/libzmq.vcproj)
  
  #get the arguments for devenv
  build_zeroMQ_command(buildCommand ${zeroMQ_sln_name})
  
  #add in a configure step that properly copies the solution files to zeroMQ source tree
  #don't use add_external_project that is a unix helper cuurently, so copy the prefix, downloaddir
  #install dir and git url
  add_external_project(zeroMQ
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ${zeroMQ_configure_sln}  <SOURCE_DIR>/builds/msvc/${zeroMQ_sln_name}
    BUILD_COMMAND ${buildCommand} 
    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo
    )

  #add in a custom pre build, post configure step to install the proper libzmq vcproj
  ExternalProject_Add_Step(zeroMQ add64BitSupportToSolution
    COMMAND ${CMAKE_COMMAND} -E copy ${zeroMQ_vcproj} <SOURCE_DIR>/builds/msvc/libzmq
    DEPENDERS build
    DEPENDEES configure
    )

  #add in a custom pre build step that upgrades the solution file
  ExternalProject_Add_Step(zeroMQ upgradeSLN
    COMMAND ${CMAKE_MAKE_PROGRAM} <SOURCE_DIR>/builds/msvc/${zeroMQ_sln_name} /upgrade 
    DEPENDERS build
    DEPENDEES add64BitSupportToSolution
    )

  #add in a custom post install command that copy the zeroMQ dll and headers to the install directory
  ExternalProject_Add_Step(zeroMQ installDll
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libzmq.dll <INSTALL_DIR>/lib/libzmq.dll
    DEPENDEES install
    )

  zeroMQ_libDir(zeroLibDir)
  ExternalProject_Add_Step(zeroMQ installLib
    COMMAND ${CMAKE_COMMAND} -E copy ${zeroLibDir} <INSTALL_DIR>/lib/libzmq.lib
    DEPENDEES install
    )

  #install header files
  ExternalProject_Add_Step(zeroMQ installZmqHeader
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/zmq.h <INSTALL_DIR>/include/zmq.h
    DEPENDEES install
    )
  ExternalProject_Add_Step(zeroMQ installZmqCPPHeader
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/zmq.hpp <INSTALL_DIR>/include/zmq.hpp
    DEPENDEES install
    )
  ExternalProject_Add_Step(zeroMQ installZmqUtilHeader
    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/zmq_utils.h <INSTALL_DIR>/include/zmq_utils.h
    DEPENDEES install
    )      

elseif()
  add_external_project(zeroMQ
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>)
endif(MSVC)


