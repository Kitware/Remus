
#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
  ActiveWorkersState.h
	JobQueue.h
  WorkerFactory.h
  uuidHelper.h
	)

meshserver_private_headers(${headers})
