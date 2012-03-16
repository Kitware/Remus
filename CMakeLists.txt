cmake_minimum_required (VERSION 2.8)
project(MeshServer)
find_project(ZeroMQ REQUIRED)

set(SRCS
   server.h
   worker.h
   main.cxx
   )

include_directories(${ZeroMQ_INCLUDE_DIRS})

add_executable(MeshServer ${SRCS})

target_link_libraries(MeshServer ${ZeroMQ_LIBRARIES})
