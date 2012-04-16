
include_directories(${Boost_INCLUDE_DIRS})

set(SRCS_Server
   serverMain.cxx
   )

add_executable(TestMeshServer ${SRCS_Server} ${COMMON_SRCS})
target_link_libraries(TestMeshServer ${ZeroMQ_LIBRARIES} MeshServerServer)

add_subdirectory(BasicExample)
add_subdirectory(Omicron)
