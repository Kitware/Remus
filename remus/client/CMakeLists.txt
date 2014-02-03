project(Remus_Client)

set(protocol_headers
    Job.h
    JobRequest.h
    JobContent.h
    JobMeshRequirements.h
    JobResult.h
    JobStatus.h
    JobSubmission.h
    )

set(protocol_srcs
    JobMeshRequirements.cxx
    JobSubmission.cxx
    )

#setup the protocol library for the client
add_library(RemusClientProtocol
            ${protocol_srcs} ${protocol_headers})

target_link_libraries(RemusClientProtocol
                      LINK_PUBLIC RemusCommon
                      )

set(headers
    Client.h
    ServerConnection.h
    )

set(srcs
    Client.cxx
    )

#setup the client side api library which uses the protocol library
add_library(RemusClient ${srcs} ${headers})
target_link_libraries(RemusClient
                      LINK_PRIVATE RemusClientProtocol
                      )

#create the export header symbol exporting
remus_export_header(RemusClient ClientExports.h)

#install the client protocol library
remus_install_library(RemusClientProtocol)

#install the client library
remus_install_library(RemusClient)

#install the headers
remus_public_headers(${headers} ${protocol_headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusClient)
export(TARGETS RemusClient RemusCommon RemusClientProtocol sysTools
               FILE RemusClient-exports.cmake)


if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
