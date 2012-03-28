#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(common_headers
  Common/jobMessage.h
  Common/jobResponse.h
  Common/meshServerGlobals.h
  Common/zmqHelper.h
  )

set_source_files_properties(${common_headers}
	PROPERTIES HEADER_FILE_ONLY TRUE
)
