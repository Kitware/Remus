
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

set(SRCS_Workers
  workerMain.cxx
  )

add_executable(BasicWorker ${SRCS_Workers})
target_link_libraries(BasicWorker RemusWorker RemusCommon)
Register_Mesh_Worker(BasicWorker "Edges" "Mesh2D")
