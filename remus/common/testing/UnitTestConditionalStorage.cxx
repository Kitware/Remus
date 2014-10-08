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

#include <string>
#include <remus/common/ConditionalStorage.h>
#include <remus/common/FileHandle.h>

#include <remus/testing/Testing.h>


int UnitTestConditionalStorage(int, char *[])
{
  //construct an empty conditional storage
  remus::common::ConditionalStorage empty;
  REMUS_ASSERT( (empty.size() == 0) );
  REMUS_ASSERT( (empty.get() == NULL) );

  std::string empty_string;
  remus::common::ConditionalStorage empty_str(empty_string);
  REMUS_ASSERT( (empty_str.size() == 0) );
  REMUS_ASSERT( (empty_str.get() == NULL) );

  empty.swap(empty_str);
  REMUS_ASSERT( (empty.size() == 0) );
  REMUS_ASSERT( (empty_str.size() == 0) );
  REMUS_ASSERT( (empty.get() == NULL) );
  REMUS_ASSERT( (empty_str.get() == NULL) );


  std::string content("Copyright (c) Kitware, Inc.");
  remus::common::ConditionalStorage t;
  {
    remus::common::ConditionalStorage temp(content);
    t.swap( temp );
  }

  REMUS_ASSERT( (t.size() == 27) );
  REMUS_ASSERT( ( *(t.get()) == 'C' ) );

  remus::common::ConditionalStorage c(content);
  REMUS_ASSERT( (c.size() == 27) )
  REMUS_ASSERT( ( *(c.get()) == 'C' ) );

  REMUS_ASSERT( ( c.get() != t.get() ) );

  remus::common::ConditionalStorage tt;
  tt = t;
  t = tt;
  REMUS_ASSERT( (tt.size() == 27) );
  REMUS_ASSERT( ( *(tt.get()) == 'C' ) );
  REMUS_ASSERT( ( tt.get() == t.get() ) );

  remus::common::ConditionalStorage s(c);
  REMUS_ASSERT( (s.size() == 27) )
  REMUS_ASSERT( ( *(s.get()) == 'C' ) )
  REMUS_ASSERT( ( s.get() == c.get() ) );

  remus::common::ConditionalStorage ss(c);
  ss = c;
  REMUS_ASSERT( (ss.size() == 27) )
  REMUS_ASSERT( ( *(ss.get()) == 'C' ) )
  REMUS_ASSERT( ( ss.get() == c.get() ) );
  REMUS_ASSERT( ( ss.get() == s.get() ) );

  c.swap(ss);
  REMUS_ASSERT( (ss.size() == 27) )
  REMUS_ASSERT( ( *(ss.get()) == 'C' ) )
  REMUS_ASSERT( ( ss.get() == c.get() ) );
  REMUS_ASSERT( ( ss.get() == s.get() ) );

  //how test File Handle
  remus::common::FileHandle fh(content);
  remus::common::ConditionalStorage fhcs(fh);
  REMUS_ASSERT( (fhcs.size() == 27) );
  REMUS_ASSERT( ( *(fhcs.get()) == 'C' ) );
  REMUS_ASSERT( ( fhcs.get() != t.get() ) );


  //test with an empty boost::shared_array
  boost::shared_array<char> empty_array;
  remus::common::ConditionalStorage emptycs(empty_array,0);
  REMUS_ASSERT( ( emptycs.size() == 0 ) );
  REMUS_ASSERT( ( emptycs.get() == NULL ) );
  REMUS_ASSERT( ( emptycs.get() == empty_array.get() ) );

  //test with a boost::shared_array point to 16mb of data
  boost::shared_array<char> allocated_array( new char[16777216] );
  remus::common::ConditionalStorage shared_mem_cs(allocated_array,16777216);
  REMUS_ASSERT( ( shared_mem_cs.size() == 16777216 ) );
  REMUS_ASSERT( ( shared_mem_cs.get() == allocated_array.get() ) );
  REMUS_ASSERT( ( shared_mem_cs.get() != empty_array.get() ) );

  return 0;
}
