
add_subdirectory(testing)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(worker)


#include Boost
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ZeroMQ_INCLUDE_DIR})

set(headers
  Job.h
  JobResult.h
  JobRequest.h
  JobStatus.h
  version.h
  )

#setup the exports for the library when used from an installed location
install(EXPORT Remus-targets DESTINATION lib)
