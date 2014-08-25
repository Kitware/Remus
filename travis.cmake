set(CTEST_PROJECT_NAME "Remus")

#What compiler are we using?
if("$ENV{CXX}" STREQUAL "g++")
	set(COMPILER "g++")
	message("Using g++.")
elseif("$ENV{CXX}" STREQUAL "clang++")
	set(COMPILER "clang++")
	message("Using clang++")
else()
	set(COMPILER "unknown")
endif()

#Setting our build name and build configuration.
string(SUBSTRING "$ENV{TRAVIS_COMMIT}" 0 7 SHA)
set(CTEST_BUILD_NAME "${COMPILER}-$ENV{BUILD_CONFIG}-${SHA}")

#Setting name of dashboard.
set(CTEST_SITE "travis.travis-ci")

#Setting directory paths.
set (CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}")
set (CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")

#Setting release/debug based on environment variable:
set(CTEST_BUILD_CONFIGURATION $ENV{BUILD_CONFIG})
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")

#if the BUILD_CONFIG is debug enable coverage
set(enable_coverage OFF)
if(CTEST_BUILD_CONFIGURATION STREQUAL "Debug" AND ${COMPILER} STREQUAL "g++")
  set(enable_coverage ON)
endif()


#Write initial cache:
file(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
Remus_ENABLE_COVERAGE:BOOL=${enable_coverage}
Remus_NO_SYSTEM_BOOST:BOOl=OFF
")

#Use Travis track:
ctest_start(Experimental TRACK "Travis" . )

#Make sure we fetch correctly.
set(CTEST_GIT_COMMAND "/usr/bin/git")
set(UPDATE_COMMAND ${CTEST_GIT_COMMAND})

#when travis is building a pull request the env variable TRAVIS_PULL_REQUEST
#is set, while when building from a branch the env variable TRAVIS_BRANCH
#is set.
#So we have to look at both of these variables to determine what the proper
#ref would be. If TRAVIS_PULL_REQUEST is false, we fall back to TRAVIS_BRANCH
set(pull_request_branch "$ENV{TRAVIS_PULL_REQUEST}")
set(requested_branch "$ENV{TRAVIS_BRANCH}")

set(fetch_path "+refs/pull/${pull_request_branch}/merge")
if(pull_request_branch STREQUAL "false")
	#fall back to using the TRAVIS_BRANCH
	set(fetch_path "+refs/heads/${requested_branch}")
endif()

message(STATUS "TRAVIS_PULL_REQUEST: ${pull_request_branch}")
message(STATUS "TRAVIS_BRANCH: ${requested_branch}")

#Mirror Travis' checkout process:
set(CTEST_GIT_UPDATE_OPTIONS "origin ${fetch_path}")
set(CTEST_UPDATE_OPTIONS ${CTEST_GIT_UPDATE_OPTIONS})

ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}")
message("Updated.")
ctest_configure(SOURCE "${CTEST_SOURCE_DIRECTORY}" BUILD "${CTEST_BINARY_DIRECTORY}" APPEND)
message("Configured.")
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND)
message("Built.")
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND PARALLEL_LEVEL 6 SCHEDULE_RANDOM ON)
message("Tested.")

#For some reason, CTest doesn't automatically figure this out.  Manually set that we want gcov.
set(CTEST_COVERAGE_COMMAND "gcov")

#try using "-p" to preserve full paths to handle the fact that we have
#to files named Job.h/Job.cxx
set(COVERAGE_EXTRA_FLAGS "-l -p")

#Set coverage exclusion
set(CTEST_CUSTOM_COVERAGE_EXCLUDE
	"${CTEST_CUSTOM_COVERAGE_EXCLUDE}"
	"/thirdparty/kwsys"
	"/thirdparty/cJson"
	"/remus/testing"
	"/remus/common/testing"
	"/remus/client/testing"
	"/remus/server/testing"
	"/remus/proto/testing"
	"/remus/worker/testing"
)

ctest_coverage(BUILD "${CTEST_BINARY_DIRECTORY}" APPEND)
message("Computed coverage.")
ctest_submit()
message("Submitted.")
