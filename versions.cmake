# This maintains the links for all sources used by this superbuild.
# Simply update this file to change the revision.
# One can use different revision on different platforms.
# e.g.
# if (UNIX)
#   ..
# else (APPLE)
#   ..
# endif()


add_revision(meshserver
  GIT_REPOSITORY "git://kwsource.kitwarein.com/meshserver/meshserver.git")


add_revision(boost
  URL "http://www.vtk.org/files/support/boost-1.45.1-cmake.tar.gz"
  URL_MD5 aec16c22af72afbcf96aad03f2187e8b
  )

if( NOT WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.tar.gz"
    URL_MD5 1b11aae09b19d18276d0717b2ea288f6)
elseif(WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.zip"
    URL_MD5 f95b6451b10fdd1416848bb3118c8380)
endif()
