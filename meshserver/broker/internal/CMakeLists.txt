
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
	JobQueue.h
	)

meshserver_public_headers(${headers})