
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(internal)

set(headers
    Broker.h
    )
set(broker_srcs
   ${headers}
   Broker.cxx
   )

add_library(MeshServerBroker ${broker_srcs} ${headers})
target_link_libraries(MeshServerBroker ${ZeroMQ_LIBRARIES})

meshserver_public_headers(${headers})
