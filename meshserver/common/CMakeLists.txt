project(MeshServer_Common)

#include the build directory for the export header
set(MeshServer_Common_INCLUDE_DIRS
  ${ZeroMQ_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  )

set(headers
  ExecuteProcess.h
  JobMessage.h
  JobResponse.h
  meshServerGlobals.h
  zmqHelper.h
  zmqTraits.h
  )

set(common_srcs
  ExecuteProcess.cxx
  )

add_library(MeshServerCommon ${common_srcs} ${headers})
target_link_libraries(MeshServerCommon ${ZeroMQ_LIBRARIES} sysTools)

meshserver_set_includes(MeshServerCommon ${MeshServer_Common_INCLUDE_DIRS})

#create the export header symbol exporting
meshserver_export_header(MeshServerCommon CommonExports.h)

#install the library
meshserver_install_library(MeshServerCommon)

#install the headers
meshserver_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE MeshServerCommon)
export(TARGETS MeshServerCommon sysTools FILE MeshServerCommon-exports.cmake)
