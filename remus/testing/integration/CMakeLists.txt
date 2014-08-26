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

set(unit_tests
  AlwaysAcceptServer.cxx
  QueryIOTypes.cxx
  SimpleJobFlow.cxx
  TerminateQueuedJob.cxx
  )

remus_integration_tests(SOURCES ${unit_tests}
                        LIBRARIES ${Boost_LIBRARIES})
