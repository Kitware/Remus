
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
	JobQueue.h
	)

set_source_files_properties(${headers}
	PROPERTIES HEADER_FILE_ONLY TRUE)