project(CoreGenWorker)

set(worker_srcs worker_main.cxx
                worker.h        worker.cxx )

add_executable(CoreGenWorker ${worker_srcs})
target_link_libraries(CoreGenWorker RemusWorker ${Boost_LIBRARIES})
target_include_directories(CoreGenWorker
                           PRIVATE ${TRIANGLE_INCLUDE_DIRS}
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../" )

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)

remus_register_mesh_worker(CoreGenWorker
                           INPUT_TYPE "Mesh3D"
                           OUTPUT_TYPE "Mesh3D"
                           NO_INSTALL)

