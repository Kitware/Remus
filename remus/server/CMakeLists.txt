project(Remus_Server)

add_subdirectory(detail)

set(headers
    Server.h
    WorkerFactory.h
	  ServerPorts.h
    )

set(server_srcs
   detail/ActiveJobs.cxx
   detail/JobQueue.cxx
   detail/WorkerPool.cxx
   detail/SocketMonitor.cxx
   Server.cxx
   ServerPorts.cxx
   WorkerFactory.cxx
   )

#include cjson
include_directories("${Remus_SOURCE_DIR}/thirdparty/cJson/")

add_library(RemusServer ${server_srcs} ${headers})
target_link_libraries(RemusServer
                      LINK_PUBLIC RemusCommon
                      LINK_PRIVATE RemusProto
                                   remuscJSON
                                   ${Boost_LIBRARIES}
                                   ${CMAKE_THREAD_LIBS_INIT}
                                   )

#create the export header symbol exporting
remus_export_header(RemusServer ServerExports.h)

#install the library
remus_install_library(RemusServer)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusServer)
export(TARGETS RemusServer RemusProto RemusCommon sysTools
               FILE Remus-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
