project(Remus_Server)

add_subdirectory(detail)

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
target_link_libraries(RemusServer
                      LINK_PUBLIC RemusCommon ${Boost_LIBRARIES})

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
