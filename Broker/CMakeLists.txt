
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(broker_headers
	jobQueue.h
	)

set_source_files_properties(broker_headers
	PROPERTIES HEADER_FILE_ONLY TRUE)

set(broker_srcs
   broker.h
   broker.cxx
   )

add_library(MeshServerBroker ${broker_srcs} ${broker_headers})
target_link_libraries(MeshServerBroker ${ZeroMQ_LIBRARIES})