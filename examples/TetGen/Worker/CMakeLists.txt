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
remus_register_mesh_worker(TetGenWorker
                           INPUT_TYPE "PiecewiseLinearComplex"
                           OUTPUT_TYPE "Mesh3D")
