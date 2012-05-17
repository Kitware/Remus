project(MeshServer_Server)

#include the include directories from the common library
meshserver_get_includes(MeshServerCommon CommonIncludes)

add_subdirectory(internal)

set(headers
    Server.h
    WorkerFactory.h
    )
set(server_srcs
   Server.cxx
   WorkerFactory.cxx
   )


add_library(MeshServerServer ${server_srcs} ${headers})
target_link_libraries(MeshServerServer ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES} MeshServerCommon)

meshserver_set_includes(MeshServerServer ${CommonIncludes})

#create the export header symbol exporting
meshserver_export_header(MeshServerServer ServerExports.h)

#install the library
meshserver_install_library(MeshServerServer)

#install the headers
meshserver_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE MeshServerServer)
export(TARGETS MeshServerServer MeshServerCommon sysTools FILE MeshServerServer-exports.cmake)
