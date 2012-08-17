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
  GIT_TAG 4f1735eb810960e)

if( NOT WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.tar.gz"
    URL_MD5 1b11aae09b19d18276d0717b2ea288f6)

  add_revision(boost
    URL "http://sourceforge.net/projects/boost/files/boost/1.49.0/boost_1_49_0.tar.gz/download"
    URL_MD5 e0defc8c818e4f1c5bbb29d0292b76ca
    )

elseif(WIN32)
  add_revision(zeroMQ
    URL "http://download.zeromq.org/zeromq-2.2.0.zip"
    URL_MD5 f95b6451b10fdd1416848bb3118c8380)

  add_revision(boost
    URL "http://sourceforge.net/projects/boost/files/boost/1.49.0/boost_1_49_0.zip/download"
    URL_MD5 854dcbbff31b896c85c38247060b7713
    )
endif()
