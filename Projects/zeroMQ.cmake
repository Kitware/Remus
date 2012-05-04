
add_external_project(zeroMQ
  CONFIGURE_COMMAND <SOURCE_DIR>/configure
    --prefix=<INSTALL_DIR>
)

ExternalProject_Get_Property(zeroMQ install_dir)
add_project_property(zeroMQ ZeroMQ_ROOT_DIR ${install_dir})
