
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(BasicWorker ${SRCS_Workers})
target_link_libraries(BasicWorker RemusWorker RemusCommon)
remus_register_mesh_worker(BasicWorker
                           INPUT_TYPE "Edges"
                           OUTPUT_TYPE "Mesh2D"
                           NO_INSTALL)
