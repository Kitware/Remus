project(TriangleWorker)

#make the tetgen worker executable
set(worker_srcs
    worker_main.cxx
    TriangleWorker.cxx
    )

add_executable(TriangleWorker ${worker_srcs})
target_link_libraries(TriangleWorker RemusWorker ${TRIANGLE_LIBRARIES})
target_include_directories(TriangleWorker
                           PRIVATE ${TRIANGLE_INCLUDE_DIRS}
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../" )

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
Register_Mesh_Worker(TriangleWorker "RAW EDGES" "2D")
