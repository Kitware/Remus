
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(internal)

set(headers
    Broker.h
    WorkerFactory.h
    )
set(broker_srcs
   Broker.cxx
   WorkerFactory.cxx
   )

add_library(MeshServerBroker ${broker_srcs} ${headers})
target_link_libraries(MeshServerBroker ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES} MeshServerCommon)

meshserver_public_headers(${headers})
