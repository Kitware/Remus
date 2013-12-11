project(TetGenWorker)

#make the tetgen worker executable
set(worker_srcs
    worker_main.cxx
    TetGenWorker.cxx
    )

add_executable(TetGenWorker ${worker_srcs})
target_link_libraries(TetGenWorker
                TetGenRuntime RemusWorker ${TETGEN_LIBRARIES} )
target_include_directories(TetGenWorker
                           PRIVATE ${TETGEN_INCLUDE_DIRS})

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
Register_Mesh_Worker(TetGenWorker "PIECWISE LINEAR COMPLEX" "3D")
