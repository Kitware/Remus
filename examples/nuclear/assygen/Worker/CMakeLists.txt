project(AssyGenWorker)

set(worker_srcs
    worker_main.cxx
    worker.h
    worker.cxx
    )

add_executable(AssyGenWorker ${worker_srcs})
target_link_libraries(AssyGenWorker RemusWorker)
target_include_directories(AssyGenWorker
                           PRIVATE ${Boost_INCLUDE_DIRS}
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../" )

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
Register_Mesh_Worker(AssyGenWorker "SceneFile" "Model")
