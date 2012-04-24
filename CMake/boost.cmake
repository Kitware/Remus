#Boost bjam version
add_external_project(boost
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh
  BUILD_COMMAND <SOURCE_DIR>/b2 --prefix=<INSTALL_DIR> --with-filesystem --with-system --with-thread install
  INSTALL_COMMAND pwd
)
