#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
  ExecuteProcess.h
  JobDetails.h
  JobResult.h
  JobStatus.h
  meshServerGlobals.h
  zmqHelper.h
  )

set(common_srcs
  ExecuteProcess.cxx
  )

add_library(MeshServerCommon ${common_srcs} ${headers})
target_link_libraries(MeshServerCommon ${ZeroMQ_LIBRARIES} sysTools)

meshserver_public_headers(${headers})
