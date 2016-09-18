project(Remus_Client)

set(headers
    Client.h
    ServerConnection.h
    )

set(srcs
    Client.cxx
    ServerConnection.cxx
    )

#setup the client side api library which uses the protocol library
add_library(RemusClient ${srcs} ${headers})
target_link_libraries(RemusClient
                      LINK_PUBLIC RemusProto
                      )

#disable checked iterators in RemusClient
if(MSVC)
  target_compile_definitions(RemusClient PRIVATE _SCL_SECURE_NO_WARNINGS)
endif()

#create the export header symbol exporting
remus_export_header(RemusClient ClientExports.h)

#install the client library
remus_install_library(RemusClient)

#install the headers
remus_public_headers(${headers} ${protocol_headers})

#setup the exports for the library when used from a build tree
export(TARGETS RemusClient
               RemusProto
               RemusCommon
               RemusSysTools
               FILE RemusClient-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
