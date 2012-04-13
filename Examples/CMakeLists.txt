
include_directories(${Boost_INCLUDE_DIRS})

set(SRCS_Broker
   brokerMain.cxx
   )

add_executable(TestMeshBroker ${SRCS_Broker} ${COMMON_SRCS})
target_link_libraries(TestMeshBroker ${ZeroMQ_LIBRARIES} MeshServerBroker)
