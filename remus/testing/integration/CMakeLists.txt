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
  ShareContext.cxx
  SimpleJobFlow.cxx
  TerminateQueuedJob.cxx
  )

remus_integration_tests(SOURCES ${unit_tests}
                        LIBRARIES ${Boost_LIBRARIES})


#also add other permutation for SubmitSmallJobs where
#we modify the number of workers and jobs
add_executable(IntegrationTests_SubmitJobs
               SubmitJobs.cxx)
target_link_libraries(IntegrationTests_SubmitJobs
                      LINK_PRIVATE
                        RemusClient
                        RemusWorker
                        RemusServer
                        ${Boost_LIBRARIES}
                      )

##### Helper function to easily add submit job tests
function(add_SubmitJobs_test size workers jobs)
  set(tname "SubmitJobs${workers}by${jobs}_${size}")
  add_test(NAME ${tname}
           COMMAND IntegrationTests_SubmitJobs ${size} ${workers} ${jobs})
  set_tests_properties(${tname} PROPERTIES TIMEOUT 120)
endfunction()

add_SubmitJobs_test(small 1 1)
add_SubmitJobs_test(small 2 4)
add_SubmitJobs_test(small 4 2)
add_SubmitJobs_test(small 6 60)

add_SubmitJobs_test(large 1 1)
add_SubmitJobs_test(large 2 4)
add_SubmitJobs_test(large 4 2)
