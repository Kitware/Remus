cmake_minimum_required (VERSION 2.8)
project(MeshServer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MeshServer_SOURCE_DIR}/CMake)

#ZeroMQ is used for all message passing between components.
find_package(ZeroMQ REQUIRED)

# List of Boost features used:
# * UUID Generation
# * Shared Ptr
# * Scoped Ptr
find_package(Boost 1.45.0 REQUIRED)

set(COMMON_SRCS
  Common/jobMessage.h
  Common/jobResponse.h
  Common/meshServerGlobals.h
  Common/zmqHelper.h
  )

set(SRCS_Client
   client.h
   clientMain.cxx
   )

set(SRCS_Broker
   broker.h
   broker.cxx
   brokerMain.cxx
   jobQueue.h
   )

set(SRCS_Workers
  worker.h
  workerMain.cxx
  )


#include ZeroMQ
include_directories(${ZeroMQ_INCLUDE_DIR})

#include Boost
include_directories(${Boost_INCLUDE_DIRS})

add_executable(MeshClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(MeshClient ${ZeroMQ_LIBRARIES})

add_executable(MeshBroker ${SRCS_Broker} ${COMMON_SRCS})
target_link_libraries(MeshBroker ${ZeroMQ_LIBRARIES})

add_executable(MeshWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(MeshWorker ${ZeroMQ_LIBRARIES})
