project(Remus_Proto)

set(headers
    conversionHelpers.h
    Job.h
    JobContent.h
    JobProgress.h
    JobRequirements.h
    JobResult.h
    JobStatus.h
    JobSubmission.h
    zmqSocketIdentity.h
    zmqSocketInfo.h
    zmqTraits.h
    )

#these are headers that don't need to be installed
set(private_headers
  Message.h
  Response.h
  zmq.hpp
  zmqHelper.h
  )

set(srcs
    JobContent.cxx
    JobRequirements.cxx
    JobResult.cxx
    JobSubmission.cxx
    Message.cxx
    Response.cxx
    )

#setup the protocol library
add_library(RemusProto ${srcs} ${headers})

target_link_libraries(RemusProto
                      LINK_PUBLIC RemusCommon ${ZeroMQ_LIBRARIES}
                      )

target_include_directories(RemusProto
                           PUBLIC  ${ZeroMQ_INCLUDE_DIRS}
                                   ${Boost_INCLUDE_DIRS})

#create the export header symbol exporting
remus_export_header(RemusProto ProtoExports.h)

#install the protocol library
remus_install_library(RemusProto)

#install the headers
remus_public_headers(${headers})
remus_private_headers(${private_headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusProto)
export(TARGETS RemusProto
               RemusCommon
               sysTools
               FILE RemusProto-exports.cmake)


if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
