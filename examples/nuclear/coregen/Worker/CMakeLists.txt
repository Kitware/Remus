project(CoreGenWorker)

set(worker_srcs worker_main.cxx
                worker.h        worker.cxx )

add_executable(CoreGenWorker ${worker_srcs})
target_link_libraries(CoreGenWorker RemusWorker ${TRIANGLE_LIBRARIES})
target_include_directories(CoreGenWorker
                           PRIVATE ${TRIANGLE_INCLUDE_DIRS}
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../" )

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
Register_Mesh_Worker(CoreGenWorker "Mesh3D" "Mesh3D")

