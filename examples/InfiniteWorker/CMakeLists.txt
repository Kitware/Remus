
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(InfiniteWorker ${SRCS_Workers})
target_link_libraries(InfiniteWorker RemusWorker)
Register_Mesh_Worker(InfiniteWorker "RAW EDGES" "2D")
