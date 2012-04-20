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
  GIT_REPOSITORY "http://git.kwsource.kitwarein.com/meshserver/meshserver.git")

#we use the cmake boost repo
add_revision(boost
  GIT_REPOSITORY "https://git.gitorious.org/boost/cmake.git"
  GIT_TAG "cmake-1.45.0")

if( NOT WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.tar.gz"
    URL_MD5 1b11aae09b19d18276d0717b2ea288f6)
elseif(WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.zip"
    URL_MD5 f95b6451b10fdd1416848bb3118c8380)
endif()
