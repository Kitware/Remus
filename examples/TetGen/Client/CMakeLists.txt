project(TetGenClient)

#make the tetgen client executable
#note it doesn't need to link or include tetgen
#this client is just used to verify that we can pass messages to the worker
#and get a valid result.
set(client_srcs
    client_main.cxx
    )
add_executable(TetGenClient ${client_srcs})
target_link_libraries(TetGenClient TetGenRuntime RemusClient)
