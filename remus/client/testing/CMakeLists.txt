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
  UnitTestClient.cxx
  UnitTestClientServerConnection.cxx
  )

#we need to explicitly link to zmq for client test, which
#calls zmq directly to verify that we can properly connect a client
#to an inproc socket
remus_unit_tests(SOURCES ${unit_tests}
                 LIBRARIES RemusClient ${ZeroMQ_LIBRARIES})
