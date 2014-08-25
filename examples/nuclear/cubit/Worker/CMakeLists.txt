project(CubitWorker)

set(worker_srcs worker_main.cxx
                worker.h        worker.cxx )

add_executable(CubitWorker ${worker_srcs})
target_link_libraries(CubitWorker RemusWorker ${Boost_LIBRARIES})
target_include_directories(CubitWorker
                           PRIVATE ${TRIANGLE_INCLUDE_DIRS}
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../" )

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

remus_register_mesh_worker(CubitWorker
                           INPUT_TYPE "Model"
                           OUTPUT_TYPE "Mesh3D"
                           NO_INSTALL)
