
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})


set(headers
    Worker.h
    )
set(worker_srcs
   Worker.cxx
   )

add_library(MeshServerWorker ${worker_srcs} ${headers})
target_link_libraries(MeshServerWorker ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES})

meshserver_public_headers(${headers})
