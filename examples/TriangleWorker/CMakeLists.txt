project(TriangleWorker)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(Triangle REQUIRED)

#make the triangle worker executable
set(worker_srcs
    worker_main.cxx
    TriangleWorker.cxx
    )
add_executable(TriangleWorker ${worker_srcs})

target_link_libraries(TriangleWorker RemusWorker ${TRIANGLE_LIBRARIES})
target_include_directories(TriangleWorker PRIVATE ${TRIANGLE_INCLUDE_DIRS})

#make the triangle client executable
#note it doesn't need to link or include triangle
#this client is just used to verify that we can pass messages to the worker
#and get a valid result.
set(client_srcs
    client_main.cxx
    )
add_executable(TriangleTestClient ${client_srcs})
target_link_libraries(TriangleTestClient ${ZeroMQ_LIBRARIES})


#write out the worker registration file for the default worker factory
include(${Remus_SOURCE_DIR}/CMake/RemusRegisterWorker.cmake)
Register_Mesh_Worker(TriangleWorker "RAW EDGES" "2D")
