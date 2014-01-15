project(Remus_Common)

set(headers
  ExecuteProcess.h
  Message.h
  MeshIOType.h
  Response.h
  SignalCatcher.h
  remusGlobals.h
  zmq.hpp
  zmqHelper.h
  zmqTraits.h
  )

set(common_srcs
  ExecuteProcess.cxx
  )

add_library(RemusCommon ${common_srcs} ${headers})
target_link_libraries(RemusCommon
                      LINK_PUBLIC ${ZeroMQ_LIBRARIES}
                      LINK_PRIVATE sysTools
                      )
target_include_directories(RemusCommon
                           PUBLIC  ${ZeroMQ_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                           PRIVATE ${sysTools_BINARY_DIR} )

#create the export header symbol exporting
remus_export_header(RemusCommon CommonExports.h)

#install the library
remus_install_library(RemusCommon)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusCommon)
export(TARGETS RemusCommon sysTools FILE RemusCommon-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
