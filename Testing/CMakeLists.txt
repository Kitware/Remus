
set(SRCS_Client
   clientMain.cxx
   )

set(SRCS_Broker
   brokerMain.cxx
   )

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(MeshClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(MeshClient ${ZeroMQ_LIBRARIES})

add_executable(MeshBroker ${SRCS_Broker} ${COMMON_SRCS})
target_link_libraries(MeshBroker ${ZeroMQ_LIBRARIES})

add_executable(MeshWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(MeshWorker ${ZeroMQ_LIBRARIES})
