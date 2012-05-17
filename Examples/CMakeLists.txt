
meshserver_get_includes(MeshServerServer WorkIncludes)
include_directories(${WorkIncludes})

set(SRCS_Server
   serverMain.cxx
   )

add_executable(TestMeshServer ${SRCS_Server} ${COMMON_SRCS})
target_link_libraries(TestMeshServer MeshServerServer)

add_subdirectory(BasicExample)
add_subdirectory(Omicron)
