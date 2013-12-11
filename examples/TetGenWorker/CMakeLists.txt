project(TetGenWorker)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(TetGen REQUIRED)


#make the tetgen worker executable
set(worker_srcs
    worker_main.cxx
    TetGenWorker.cxx
    TetGenInput.cxx
    )

add_executable(TetGenWorker ${worker_srcs})
target_link_libraries(TetGenWorker
                      RemusWorker ${TETGEN_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(TetGenWorker
                           PRIVATE ${TETGEN_INCLUDE_DIRS})

#make the tetgen client executable
#note it doesn't need to link or include tetgen
#this client is just used to verify that we can pass messages to the worker
#and get a valid result.
set(client_srcs
    client_main.cxx
    TetGenInput.cxx
    )
add_executable(TetGenClient ${client_srcs})
target_link_libraries(TetGenClient RemusClient ${Boost_LIBRARIES})

#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
Register_Mesh_Worker(TetGenWorker "PIECWISE LINEAR COMPLEX" "3D")


#copy the input testing data to the build directory
file(COPY data/pmdc.node data/pmdc.poly
     DESTINATION ${EXECUTABLE_OUTPUT_PATH}
     )
