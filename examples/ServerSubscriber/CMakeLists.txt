
set(client
   serversub.cxx
   )

#include cjson
include_directories("${Remus_SOURCE_DIR}/thirdparty/cJson/")

add_executable(ServerSubscriber ${client})
target_link_libraries(ServerSubscriber LINK_PUBLIC RemusProto LINK_PRIVATE remuscJSON)

