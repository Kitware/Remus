
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(internal)

set(broker_srcs
   broker.h
   broker.cxx
   )

add_library(MeshServerBroker ${broker_srcs} ${broker_headers})
target_link_libraries(MeshServerBroker ${ZeroMQ_LIBRARIES})