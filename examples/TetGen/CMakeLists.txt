list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(TetGen REQUIRED)

set(runtime_srcs
    TetGenInput.cxx
    )

add_library(TetGenRuntime STATIC ${runtime_srcs})
target_link_libraries(TetGenRuntime LINK_PRIVATE ${Boost_LIBRARIES})
target_include_directories(TetGenRuntime
                INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(Client)
add_subdirectory(Worker)

#copy the input testing data to the build directory
file(COPY data/pmdc.node data/pmdc.poly
     DESTINATION ${EXECUTABLE_OUTPUT_PATH}
     )
