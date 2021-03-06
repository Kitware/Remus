//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#include <remus/common/Timer.h>
#include <remus/testing/Testing.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/date_time/posix_time/posix_time.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace
{

void verify_elapsed()
{
  //just verify that we have a copy constructor
  boost::posix_time::ptime before_init =
                              boost::posix_time::microsec_clock::local_time();
  remus::common::Timer timer;
  boost::int64_t milliE = 0;
  do
    {
    milliE = timer.elapsed();
    }
  while( milliE < 20 );

  boost::posix_time::ptime after_elapsed =
                              boost::posix_time::microsec_clock::local_time();

  boost::posix_time::time_duration delta = after_elapsed - before_init;

  std::cout << "milliE: " << milliE << std::endl;
  std::cout << "delta.total_milliseconds(): " << delta.total_milliseconds() << std::endl;
  REMUS_ASSERT( (milliE <= delta.total_milliseconds()) );
}

void verify_reset()
{
  //just verify that we have a copy constructor
  remus::common::Timer timer;

  //burn 125 milliseconds
  boost::int64_t milliE = 0;
  do
    {
    milliE = timer.elapsed();
    }
  while( milliE < 125 );


  boost::posix_time::ptime before_reset =
                              boost::posix_time::microsec_clock::local_time();

  timer.reset();
  milliE = timer.elapsed();

  boost::posix_time::ptime after_elapsed =
                              boost::posix_time::microsec_clock::local_time();

  boost::posix_time::time_duration delta = after_elapsed - before_reset;

  std::cout << "milliE: " << milliE << std::endl;
  std::cout << "delta.total_milliseconds(): " << delta.total_milliseconds() << std::endl;
  REMUS_ASSERT( (milliE <= delta.total_milliseconds()) );
}

}

int UnitTestTimer(int, char *[])
{
  verify_elapsed();
  verify_reset();
  return 0;
}
