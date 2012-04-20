
add_external_project(meshserver
  DEPENDS boost zeroMQ
  CMAKE_ARGS
    -DBoost_INCLUDE_DIR:PATH=<INSTALL_DIR>
    -DZeroMQ_INCLUDE_DIR:PATH=<INSTALL_DIR>
    -DCMAKE_INSTALL_PREFIX:Path=<INSTALL_DIR>
    )
