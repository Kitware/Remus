
set(SRCS_Client
   clientMain.cxx
   )

set(SRCS_Broker
   brokerMain.cxx
   )

set(SRCS_Workers
  workerMain.cxx
  )

include_directories(${Boost_INCLUDE_DIRS})

add_executable(TestMeshClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(TestMeshClient ${ZeroMQ_LIBRARIES})

add_executable(TestMeshBroker ${SRCS_Broker} ${COMMON_SRCS})
target_link_libraries(TestMeshBroker ${ZeroMQ_LIBRARIES} MeshServerBroker)

add_executable(TestMeshWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(TestMeshWorker ${ZeroMQ_LIBRARIES} ${Boost_LIBRARIES})
