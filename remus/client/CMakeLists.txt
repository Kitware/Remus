project(Remus_Client)

set(headers
    Client.h
    Job.h
    JobRequest.h
    JobContent.h
    JobMeshRequirements.h
    JobResult.h
    JobStatus.h
    JobSubmission.h
    ServerConnection.h
    )

set(client_srcs
    Client.cxx
    JobContent.cxx
    JobMeshRequirements.cxx
    JobSubmission.cxx
    )

add_library(RemusClient ${client_srcs} ${headers})
target_link_libraries(RemusClient
                      LINK_PUBLIC RemusCommon
                      )

#create the export header symbol exporting
remus_export_header(RemusClient ClientExports.h)

#install the library
remus_install_library(RemusClient)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusClient)
export(TARGETS RemusClient RemusCommon sysTools FILE RemusClient-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
