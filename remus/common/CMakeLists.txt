project(Remus_Common)

#include the build directory for the export header
set(Remus_Common_INCLUDE_DIRS
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

add_library(RemusCommon ${common_srcs} ${headers})
target_link_libraries(RemusCommon ${ZeroMQ_LIBRARIES} sysTools)

remus_set_includes(RemusCommon ${Remus_Common_INCLUDE_DIRS})

#create the export header symbol exporting
remus_export_header(RemusCommon CommonExports.h)

#install the library
remus_install_library(RemusCommon)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusCommon)
export(TARGETS RemusCommon sysTools FILE RemusCommon-exports.cmake)
