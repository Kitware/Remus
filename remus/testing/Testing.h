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
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#ifndef remus_testing_Testing_h
#define remus_testing_Testing_h

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>

//suppress warnings inside boost headers for gcc and clang
#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

namespace remus {
namespace testing {

namespace {

static char* full_character_set()
{
  static bool random_setup = false;
  if(!random_setup)
    {
    random_setup = true;
    //setup the random number generator
    std::srand(static_cast<unsigned int>(std::time(0)));
    }

  static char charSet[256] =
                { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                  40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                  50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                  60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
                  70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                  80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
                  90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
                  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                  110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
                  120, 121, 122, 123, 124, 125, 126,
                  -1, -2, -3, -4, -5, -6, -7, -8, -9,
                  -10, -11, -12, -13, -14, -15, -16, -17, -18, -19,
                  -20, -21, -22, -23, -24, -25, -26, -27, -28, -29,
                  -30, -31, -32, -33, -34, -35, -36, -37, -38, -39,
                  -40, -41, -42, -43, -44, -45, -46, -47, -48, -49,
                  -50, -51, -52, -53, -54, -55, -56, -57, -58, -59,
                  -60, -61, -62, -63, -64, -65, -66, -67, -68, -69,
                  -70, -71, -72, -73, -74, -75, -76, -77, -78, -79,
                  -80, -81, -82, -83, -84, -85, -86, -87, -88, -89,
                  -90, -91, -92, -93, -94, -95, -96, -97, -98, -99,
                  -100, -101, -102, -103, -104, -105, -106, -107, -108, -109,
                  -110, -111, -112, -113, -114, -115, -116, -117, -118, -119,
                  -120, -121, -122, -123, -124, -125, -126, -127, -128
                };
  return charSet;
}

struct char_gen
{
  std::string T;
  char_gen(const std::string& t):T(t){}
  char operator()() const
  { return T[std::rand() % (T.size()-1) ]; }

};

inline std::string CharacterGenerator(const std::string& charset,
                                      std::size_t length)
{

  //first thing we need to do is construct a large string
  //which has a collection of the values from the charset. We want
  //duplicate characters from the charset to occur
  std::string sampling(4096,0);
  std::generate_n(sampling.begin(),sampling.size(),char_gen(charset) );

  //now we need to take that sampling and copy it into the charset
  //first we copy the remainder, or in the case of short strings
  //this will fill the entire string
  std::string result(length,0);
  if(length > 0) //have to handle asking for an empty string gen
    {
    std::size_t remainder = length % sampling.size();
    std::copy(sampling.begin(),sampling.begin()+remainder,result.begin());

    std::size_t times_to_copy = length / sampling.size();
    std::string::iterator pos = result.begin()+remainder;
    for(std::size_t i=0; i < times_to_copy; ++i, pos += sampling.size())
      {
      std::copy(sampling.begin(),sampling.end(),pos);
      }
    }
  return result;
}

}

inline std::string BinaryDataGenerator(std::size_t length)
{
  std::string characters(full_character_set(),256);
  return CharacterGenerator(characters,length);
}

inline std::string AsciiStringGenerator(std::size_t length)
{
  //we only want the subset of visible ascii characters
  char* full_chars = full_character_set();
  char* start = full_chars+32;
  const std::size_t char_set_length = 95;

  std::string characters(start,char_set_length);
  std::string result = CharacterGenerator(characters,length);
  return result;
}

inline boost::uuids::uuid UUIDGenerator()
{
  static boost::mt19937 ran;
  static boost::uuids::basic_random_generator<boost::mt19937> generator(&ran);
  return generator();
}

inline std::string UniqueString()
{
  boost::uuids::uuid i = UUIDGenerator();
  return boost::lexical_cast<std::string>(i);
}

}
}


//These asserts are meant to be in the global namespace

/// Asserts a condition for a test to pass. A passing condition is when \a
/// condition resolves to true. If \a condition is false, then the test is
/// aborted and failure is returned.
#define REMUS_ASSERT(condition) \
  if(!condition) \
    { std::cerr << "Error at file: "  << __FILE__ << std::endl; \
    std::cerr <<  "On line: " <<  __LINE__ << std::endl; exit(1); }


///  A passing condition is when \a condition resolves to true.
///  If \a condition is false, then the valid_flag is set to false
///  and we output to cerr which line we failed on.
///  If the \a condition is true the valid flag is not modified
#define REMUS_VALID(condition, valid_flag) \
  valid_flag &= condition; \
  if(!condition) \
    { std::cerr << "Error at file: "  << __FILE__ << std::endl; \
    std::cerr <<  "On line: " <<  __LINE__ << std::endl; \
    }

#endif //remus_testing_Testing
