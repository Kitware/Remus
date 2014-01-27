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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <remus/client/JobResult.h>
#include <remus/testing/Testing.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <ctime>

namespace {

using namespace remus::client;
boost::uuids::random_generator generator;


std::string efficientStringGenerator(int length)
{
  std::string result;
  result.resize(length);

  std::string charset("abcdefghijklmnopqrstuvwxyz");
  std::random_shuffle(charset.begin(),charset.end());

  const std::size_t remainder = length % charset.length();
  const std::size_t times_to_copy = length / charset.length();

  //fill the remainder in first
  typedef std::string::iterator it;
  it start = result.begin();
  std::copy(charset.begin(), charset.begin()+remainder,start);

  std::random_shuffle(charset.begin(),charset.end());
  start += remainder;
  for(int i=0; i < times_to_copy; ++i, start+=charset.length())
    { std::copy(charset.begin(), charset.end(), start); }
  return result;
}

void validate_serialization(JobResult s)
{
  std::string temp = to_string(s);
  JobResult from_string = to_JobResult(temp);

  REMUS_ASSERT( (from_string.JobId == s.JobId) );
  REMUS_ASSERT( (from_string.Data == s.Data) );
  REMUS_ASSERT( (from_string.valid() == s.valid()) );
}

void serialize_test()
{
  JobResult a(generator());
  validate_serialization(a);

  JobResult b(generator(),std::string());
  validate_serialization(b);

  JobResult c(generator(),std::string("Contents"));
  validate_serialization(c);

  JobResult d(generator(), efficientStringGenerator(10240*10240) );
  validate_serialization(d);
}

}

int UnitTestClientJobResult(int, char *[])
{
  serialize_test();
  return 0;
}