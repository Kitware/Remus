project(MeshServer_Worker)

#include the include directories from the common library
meshserver_get_includes(MeshServerCommon CommonIncludes)

set(headers
    Worker.h
    )

set(worker_srcs
   Worker.cxx
   )

add_library(MeshServerWorker ${worker_srcs} ${headers})
generate_export_header(MeshServerWorker EXPORT_FILE_NAME WorkerExports.h)
target_link_libraries(MeshServerWorker ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES})

meshserver_set_includes(MeshServerWorker ${CommonIncludes})
meshserver_install_library(MeshServerWorker)
meshserver_public_headers(${headers})
