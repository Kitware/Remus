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

if( NOT WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.tar.gz"
    URL_MD5 1b11aae09b19d18276d0717b2ea288f6)

  add_revision(boost
    URL "http://sourceforge.net/projects/boost/files/boost/1.48.0/boost_1_48_0.tar.gz/download"
    URL_MD5 313a11e97eb56eb7efd18325354631be
    )
elseif(WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.zip"
    URL_MD5 f95b6451b10fdd1416848bb3118c8380)

  add_revision(boost
    URL "http://sourceforge.net/projects/boost/files/boost/1.48.0/boost_1_48_0.zip/download"
    URL_MD5 b08fda829eec96b4f1071ce2ea6831f5
    )    
endif()
