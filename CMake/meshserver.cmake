
add_external_project(meshserver
  DEPENDS boost zeroMQ
  CMAKE_ARGS
    -DBOOST_ROOT:PATH=${BOOST_ROOT}
    -DBOOST_LIBRARYDIR:PATH=<INSTALL_DIR>/lib
    -DZeroMQ_ROOT_DIR:PATH=<INSTALL_DIR>
  )
