
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

remus_get_includes(RemusWorker WorkIncludes)
include_directories(${WorkIncludes})

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(BasicWorker ${SRCS_Workers} ${COMMON_SRCS})
target_link_libraries(BasicWorker ${ZeroMQ_LIBRARIES} RemusWorker)
Register_Mesh_Worker(BasicWorker 2D)
