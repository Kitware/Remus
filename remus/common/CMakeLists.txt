project(Remus_Common)

set(headers
    CompilerInformation.h
    ConditionalStorage.h
    ContentTypes.h
    ExecuteProcess.h
    FileHandle.h
    LocateFile.h
    MD5Hash.h
    MeshIOType.h
    MeshRegistrar.h
    MeshTypes.h
    ServiceTypes.h
    SignalCatcher.h
    SleepFor.h
    StatusTypes.h
    Timer.h
    )

#private common files that don't need
#be given to the users of remus, but
#are needed by other remus libraries
set(private_headers
    PollingMonitor.h
    ConversionHelper.h
    )

set(srcs
    MeshIOType.cxx
    ExecuteProcess.cxx
    LocateFile.cxx
    MD5Hash.cxx
    MeshRegistrar.cxx
    SignalCatcher.cxx
    SleepFor.cxx
    PollingMonitor.cxx
    Timer.cxx
    )

#setup the common library
add_library(RemusCommon ${srcs} ${headers})
target_link_libraries(RemusCommon
                      LINK_PRIVATE RemusSysTools ${Boost_LIBRARIES}
                      )
target_include_directories(RemusCommon
                           PUBLIC  ${Boost_INCLUDE_DIRS}
                                   $<BUILD_INTERFACE:${Remus_SOURCE_DIR}>
                                   $<BUILD_INTERFACE:${Remus_BINARY_DIR}>
                                   $<INSTALL_INTERFACE:include>
                           PRIVATE ${RemusSysTools_BINARY_DIR} )

if(APPLE)
  #needed for LocateFile
  find_library(COREFOUNDATION_LIBRARY CoreFoundation )
  target_link_libraries(RemusCommon LINK_PRIVATE ${COREFOUNDATION_LIBRARY} )
endif()

#create the export header symbol exporting
remus_export_header(RemusCommon CommonExports.h)

#install the libraries
remus_install_library(RemusCommon)

#install the headers
remus_public_headers(${headers})
remus_private_headers(${private_headers})

#setup the exports for the library when used from a build tree
export(PACKAGE RemusCommon)
export(TARGETS RemusCommon
               RemusSysTools
               FILE RemusCommon-exports.cmake)

if(Remus_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
