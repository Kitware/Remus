cmake_minimum_required (VERSION 2.8)
project(MeshServer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MeshServer_SOURCE_DIR}/CMake)

find_package(ZeroMQ REQUIRED)


set(SRCS_Client
   client.h
   clientMain.cxx
   )

set(SRCS_Broker
   broker.h
   brokerMain.cxx
   )

set(SRCS_Workers
  worker.h
  workerMain.cxx
  )

include_directories(${ZeroMQ_INCLUDE_DIR})

add_executable(MeshClient ${SRCS_Client})
target_link_libraries(MeshClient ${ZeroMQ_LIBRARIES})

add_executable(MeshBroker ${SRCS_Broker})
target_link_libraries(MeshBroker ${ZeroMQ_LIBRARIES})

add_executable(MeshWorker ${SRCS_Workers})
target_link_libraries(MeshWorker ${ZeroMQ_LIBRARIES})
