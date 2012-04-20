
add_external_project(meshserver
  DEPENDS boost zeroMQ
  CMAKE_ARGS
    -DBoost_INCLUDE_DIR:PATH=<INSTALL_DIR>/include
    -DZeroMQ_INCLUDE_DIR:PATH=<INSTALL_DIR>/include
    -DCMAKE_INSTALL_PREFIX:Path=<INSTALL_DIR>
    )
