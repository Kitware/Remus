project(CoreGenClient)

set(client_srcs
    client_main.cxx
    client.h
    client.cxx
    )
add_executable(CoreGenClient ${client_srcs})
target_link_libraries(CoreGenClient RemusClient)
target_include_directories(CoreGenClient
                           PRIVATE ${Boost_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/../")
