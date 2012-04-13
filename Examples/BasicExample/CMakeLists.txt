
include(${MeshServer_SOURCE_DIR}/CMake/MeshServerRegisterWorker.cmake)
include_directories(${Boost_INCLUDE_DIRS})

set(SRCS_Client
   clientMain.cxx
   )

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(BasicExampleClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(BasicExampleClient ${ZeroMQ_LIBRARIES})

add_executable(BasicExampleWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(BasicExampleWorker ${ZeroMQ_LIBRARIES} MeshServerWorker)
Register_Mesh_Worker(BasicExampleWorker 2D)
