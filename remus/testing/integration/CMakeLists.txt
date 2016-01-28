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
  DifferentConnectionTypes.cxx
  FailedJob.cxx
  QueryIOTypes.cxx
  ShareContext.cxx
  SimpleJobFlow.cxx
  TerminateMultipleRunningWorkers.cxx
  TerminateQueuedJob.cxx
  TerminateRunningJob.cxx
  TerminateRunningWorker.cxx
  )

remus_integration_tests(SOURCES ${unit_tests}
                        LIBRARIES ${Boost_LIBRARIES})

#also add other permutation for SubmitJobs where
#we modify the number of workers and jobs
add_executable(IntegrationTests_SubmitJobs
               SubmitJobs.cxx)
add_executable(IntegrationTests_SubmitJobsUsingFactory
               SubmitJobsUsingFactory.cxx)

target_link_libraries(IntegrationTests_SubmitJobs
    LINK_PRIVATE RemusClient RemusWorker RemusServer ${Boost_LIBRARIES} )
target_link_libraries(IntegrationTests_SubmitJobsUsingFactory
    LINK_PRIVATE RemusClient RemusWorker RemusServer ${Boost_LIBRARIES} )


#The integration tests use boost::asio which has an optional dependency on
#boost::regex. We need to state explicitly that we don't use boost regex
#so that MSVC doesn't try to link to it.
target_compile_definitions(IntegrationTests_remus_testing_integration PRIVATE BOOST_REGEX_NO_LIB)
target_compile_definitions(IntegrationTests_SubmitJobs PRIVATE BOOST_REGEX_NO_LIB)
target_compile_definitions(IntegrationTests_SubmitJobsUsingFactory PRIVATE BOOST_REGEX_NO_LIB)

#disable SCL and CRT warnings on windows for integration tests.
#remus_integration_tests macro adds these defines by default
if(MSVC)
  target_compile_definitions(IntegrationTests_SubmitJobs
    PRIVATE
    _SCL_SECURE_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
  )
  target_compile_definitions(IntegrationTests_SubmitJobsUsingFactory
    PRIVATE
    _SCL_SECURE_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
  )
  endif()

##### Helper function to easily add submit job tests
function(add_SubmitJobs_test size workers jobs)
  set(tname "SubmitJobs${workers}by${jobs}_${size}")
  add_test(NAME ${tname}
           COMMAND IntegrationTests_SubmitJobs ${size} ${workers} ${jobs})
  set_tests_properties(${tname} PROPERTIES TIMEOUT 300)
  #This test needs to be run serially as it binds to lots of ports and will
  #be an issue to other tests
  set_tests_properties(${tname} PROPERTIES RUN_SERIAL TRUE)
endfunction()

function(add_SubmitJobsFactory_test size workers jobs)
  set(tname "SubmitJobsFactory${workers}by${jobs}_${size}")
  add_test(NAME ${tname}
           COMMAND IntegrationTests_SubmitJobsUsingFactory ${size} ${workers} ${jobs})
  set_tests_properties(${tname} PROPERTIES TIMEOUT 300)
  #This test needs to be run serially as it binds to lots of ports and will
  #be an issue to other tests
  set_tests_properties(${tname} PROPERTIES RUN_SERIAL TRUE)
endfunction()

add_SubmitJobs_test(small 1 1)
add_SubmitJobs_test(small 2 4)
add_SubmitJobs_test(small 4 2)
add_SubmitJobs_test(small 6 32)

add_SubmitJobs_test(large 1 1)
add_SubmitJobs_test(large 2 4)
add_SubmitJobs_test(large 4 2)
add_SubmitJobs_test(large 7 38)


add_SubmitJobsFactory_test(small 4 2)
add_SubmitJobsFactory_test(small 6 33)

add_SubmitJobsFactory_test(large 4 2)
add_SubmitJobsFactory_test(large 7 38)
