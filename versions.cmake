# This maintains the links for all sources used by this superbuild.
# Simply update this file to change the revision.
# One can use different revision on different platforms.
# e.g.
# if (UNIX)
#   ..
# else (APPLE)
#   ..
# endif()


add_revision(remus
  GIT_REPOSITORY "git://public.kitware.com/Remus.git"
  GIT_TAG "master")

add_revision(boost
  URL "http://www.paraview.org/files/dependencies/boost_1_50_0.tar.gz"
  URL_MD5 dbc07ab0254df3dda6300fd737b3f264)

if( NOT WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.tar.gz"
    URL_MD5 1b11aae09b19d18276d0717b2ea288f6)

elseif(WIN32)
  #windows uses the custom cmake build of zeroMQ
  add_revision(zeroMQ
    GIT_REPOSITORY https://github.com/robertmaynard/zeromq2-x
    GIT_TAG master)

endif()
