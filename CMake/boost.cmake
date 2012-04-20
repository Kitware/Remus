#Boost bjam version
add_external_project(boost
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh
    --prefix=<INSTALL_DIR>
    --with-libraries=date_time,exception,random,regex,system,thread
  BUILD_COMMAND pwd
  INSTALL_COMMAND <SOURCE_DIR>/b2 install
)
