#Boost bjam version

#configure build call for boost. we need to do it this way
#to make sure we are in the correct working dirctory on windows
add_external_project(boost
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND "<SOURCE_DIR>/bootstrap.bat
    --prefix=<INSTALL_DIR>
    --with-libraries=filesystem,system,thread"
  BUILD_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/boostBuild.cmake
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(boost source_dir)
ExternalProject_Get_Property(boost binary_dir)
configure_file(${MeshServerSuperBuild_CMAKE_DIR}/win32/boostBuild.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/boostBuild.cmake
               @ONLY)
