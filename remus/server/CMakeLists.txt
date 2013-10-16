project(Remus_Server)

#include the include directories from the common library
remus_get_includes(RemusCommon CommonIncludes)

add_subdirectory(internal)

set(headers
    Server.h
    WorkerFactory.h
	ServerPorts.h
    )

set(server_srcs
   Server.cxx
   WorkerFactory.cxx
   )


add_library(RemusServer ${server_srcs} ${headers})
target_link_libraries(RemusServer ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES} RemusCommon)

remus_set_includes(RemusServer ${CommonIncludes})

#create the export header symbol exporting
remus_export_header(RemusServer ServerExports.h)

#install the library
remus_install_library(RemusServer)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusServer)
export(TARGETS RemusServer RemusCommon sysTools FILE Remus-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
