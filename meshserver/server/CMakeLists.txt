
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

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

meshserver_public_headers(${headers})
