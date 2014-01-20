project(AssyGenClient)

set(client_srcs
    client_main.cxx
    client.h
    client.cxx
    )
add_executable(AssyGenClient ${client_srcs})
target_link_libraries(AssyGenClient RemusClient)
target_include_directories(AssyGenClient
                           PRIVATE ${Boost_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/../")
