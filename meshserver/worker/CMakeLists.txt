project(MeshServer_Worker)

#include the include directories from the common library
meshserver_get_includes(MeshServerCommon CommonIncludes)

set(headers
    Worker.h
    ServerConnection.h
    )

set(worker_srcs
   Worker.cxx
   )

add_library(MeshServerWorker ${worker_srcs} ${headers})
target_link_libraries(MeshServerWorker ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES})

meshserver_set_includes(MeshServerWorker ${CommonIncludes})

meshserver_export_header(MeshServerWorker WorkerExports.h)
meshserver_install_library(MeshServerWorker)
meshserver_public_headers(${headers})
