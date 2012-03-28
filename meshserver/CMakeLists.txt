
add_subdirectory(internal)
add_subdirectory(broker)


#include Boost
include_directories(${Boost_INCLUDE_DIRS})

set(headers
  client.h
  )

meshserver_declare_headers(${headers})