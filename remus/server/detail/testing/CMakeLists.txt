#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

#the detail classes aren't exported classes, and don't
#have any symbols, so we need to compile them into our unit test executable
set(srcs
  ../ActiveJobs.cxx
  ../JobQueue.cxx
  ../WorkerPool.cxx
  ../SocketMonitor.cxx
  )

set(unit_tests
  UnitTestActiveJobs.cxx
  UnitTestServerJobQueue.cxx
  UnitTestUUIDHelper.cxx
  UnitTestWorkerPool.cxx
  )

remus_unit_tests( SOURCES ${unit_tests}
                  EXTRA_SOURCES ${srcs}
                  LIBRARIES RemusProto)