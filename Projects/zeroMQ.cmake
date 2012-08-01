
#we have to build in the source directory on Linux and OSX
#If the path to the src directory has a space, CMake will quote
#the path, which than the zeroMQ configure script will reject
#as it doesn't allow quoted paths to the configure script.

add_external_project(zeroMQ
  CONFIGURE_COMMAND <SOURCE_DIR>/configure
    --prefix=<INSTALL_DIR>
  BUILD_IN_SOURCE 1
)

ExternalProject_Get_Property(zeroMQ install_dir)
add_project_property(zeroMQ ZeroMQ_ROOT_DIR ${install_dir})
