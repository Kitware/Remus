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
  GIT_TAG "master"
  )

#need to update to 1.56
add_revision(boost
  URL "http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz/download"
  URL_MD5 8c54705c424513fa2be0042696a3a162
  )

add_revision(zeroMQ
  GIT_REPOSITORY "https://github.com/robertmaynard/zeromq3-x.git"
  GIT_TAG "master"
  )
