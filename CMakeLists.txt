cmake_minimum_required (VERSION 2.8)
project(MeshServer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MeshServer_SOURCE_DIR}/CMake)

find_package(ZeroMQ REQUIRED)

set(COMMON_SRCS
  Common/meshServerGlobals.h
  Common/mdp.h
  Common/zmsg.h
  Common/zhelpers.h
  )

set(SRCS_Client
   Common/mdcliapi.h
   client.h
   clientMain.cxx
   )

set(SRCS_Broker
   Common/mdbrkapi.h
   broker.h
   brokerMain.cxx
   )

set(SRCS_Workers
  Common/mdwrkapi.h
  worker.h
  workerMain.cxx
  )

include_directories(${ZeroMQ_INCLUDE_DIR})

add_executable(MeshClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(MeshClient ${ZeroMQ_LIBRARIES})

add_executable(MeshBroker ${SRCS_Broker} ${COMMON_SRCS})
target_link_libraries(MeshBroker ${ZeroMQ_LIBRARIES})

add_executable(MeshWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(MeshWorker ${ZeroMQ_LIBRARIES})
