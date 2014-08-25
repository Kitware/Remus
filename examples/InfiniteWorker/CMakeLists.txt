
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(InfiniteWorker ${SRCS_Workers})
target_link_libraries(InfiniteWorker RemusWorker RemusCommon)
remus_register_mesh_worker(InfiniteWorker
                           INPUT_TYPE "Edges"
                           OUTPUT_TYPE "Mesh2D"
                           NO_INSTALL)
