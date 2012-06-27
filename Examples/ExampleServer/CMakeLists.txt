
remus_get_includes(RemusServer WorkIncludes)
include_directories(${WorkIncludes})

set(SRCS_Server
   serverMain.cxx
   )

add_executable(TestRemus ${SRCS_Server} ${COMMON_SRCS})
target_link_libraries(TestRemus RemusServer)

add_subdirectory(BasicExample)
add_subdirectory(Omicron)
