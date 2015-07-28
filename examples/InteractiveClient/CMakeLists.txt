
set(client_two
  interactiveClient.cxx
   )

#include cjson
include_directories("${Remus_SOURCE_DIR}/thirdparty/cJson/")

add_executable(InteractiveClient ${client_two})
target_link_libraries(InteractiveClient LINK_PUBLIC RemusClient LINK_PRIVATE remuscJSON)
