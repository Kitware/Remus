
set(QT_USE_IMPORTED_TARGETS TRUE)
set(QT_USE_QTMAIN TRUE)
find_package(Qt4 COMPONENTS QtCore QtGui QtMain)

if(Qt4_FOUND)

  include(${QT_USE_FILE})

  set(srcs
   qcontrols.cxx
   qjob.cxx
   qserver.cxx
   qworker.cxx
   main.cxx
   )

  qt4_wrap_ui(UI_BUILT_SOURCES UI/qcontrols.ui)

  list(APPEND srcs ${UI_BUILT_SOURCES} ${RCS_SOURCES})

  #needed to get the mocced files
  include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

  add_executable(QtServerWorkerExample ${srcs})
  target_link_libraries(QtServerWorkerExample RemusServer RemusWorker RemusClient ${QT_LIBRARIES})

  #enable automocing
  set_target_properties(QtServerWorkerExample PROPERTIES AUTOMOC TRUE)

endif()