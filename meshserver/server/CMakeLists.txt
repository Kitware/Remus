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

meshserver_export_header(MeshServerServer ServerExports.h)
meshserver_install_library(MeshServerServer)
meshserver_public_headers(${headers})
