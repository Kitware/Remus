
add_subdirectory(common)
add_subdirectory(broker)
add_subdirectory(worker)


#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
  Client.h
  JobMessage.h
  JobResponse.h
  )

meshserver_public_headers(${headers})
