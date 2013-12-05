
set(SRCS_Client
   clientMain.cxx
   )

add_executable(BasicClient ${SRCS_Client})
target_link_libraries(BasicClient RemusClient)
