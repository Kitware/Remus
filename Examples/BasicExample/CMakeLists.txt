
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

remus_get_includes(RemusWorker WorkIncludes)
include_directories(${WorkIncludes})

set(SRCS_Client
   clientMain.cxx
   )

set(SRCS_Workers
  workerMain.cxx
  )

set(client_two
  interactiveClient.cxx
   )


add_executable(BasicExampleClient ${SRCS_Client} ${COMMON_SRCS})
target_link_libraries(BasicExampleClient ${ZeroMQ_LIBRARIES})

add_executable(BasicExampleWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(BasicExampleWorker ${ZeroMQ_LIBRARIES} RemusWorker)
Register_Mesh_Worker(BasicExampleWorker 2D)


add_executable(InteractiveClient ${client_two} ${COMMON_SRCS})
target_link_libraries(InteractiveClient ${ZeroMQ_LIBRARIES})
