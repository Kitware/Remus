project(TriangleClient)

#make the triangle client executable
#note it doesn't need to link or include triangle
#this client is just used to verify that we can pass messages to the worker
#and get a valid result.
set(client_srcs
    client_main.cxx
    )
add_executable(TriangleClient ${client_srcs})
target_link_libraries(TriangleClient RemusClient)
target_include_directories(TriangleClient
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../")
