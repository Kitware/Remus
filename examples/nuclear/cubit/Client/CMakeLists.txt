project(CubitClient)

set(client_srcs
    client_main.cxx
    client.h
    client.cxx
    )
add_executable(CubitClient ${client_srcs})
target_link_libraries(CubitClient RemusClient)
target_include_directories(CubitClient
                           PRIVATE ${Boost_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/../")
