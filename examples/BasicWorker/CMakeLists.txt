
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(BasicWorker ${SRCS_Workers})
target_link_libraries(BasicWorker RemusWorker)
Register_Mesh_Worker(BasicWorker "RAW EDGES" "2D")
