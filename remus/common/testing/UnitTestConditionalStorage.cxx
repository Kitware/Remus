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

#include <remus/testing/Testing.h>


int UnitTestConditionalStorage(int, char *[])
{
  //construct an empty conditional storage
  remus::common::ConditionalStorage empty;
  REMUS_ASSERT( (empty.size() == 0) )
  REMUS_ASSERT( (empty.get() == NULL) )

  std::string empty_string;
  remus::common::ConditionalStorage empty_str(empty_string);
  REMUS_ASSERT( (empty_str.size() == 0) )
  REMUS_ASSERT( (empty_str.get() == NULL) )

  empty.swap(empty_str);
  REMUS_ASSERT( (empty.size() == 0) )
  REMUS_ASSERT( (empty_str.size() == 0) )
  REMUS_ASSERT( (empty.get() == NULL) )
  REMUS_ASSERT( (empty_str.get() == NULL) )


  std::string content("Copyright (c) Kitware, Inc.");
  remus::common::ConditionalStorage t;
  {
    remus::common::ConditionalStorage temp(content);
    t.swap( temp );
  }

  REMUS_ASSERT( (t.size() == 27) )
  REMUS_ASSERT( ( *(t.get()) == 'C' ) )

  remus::common::ConditionalStorage c(content);
  REMUS_ASSERT( (c.size() == 27) )
  REMUS_ASSERT( ( *(c.get()) == 'C' ) )

  REMUS_ASSERT( ( c.get() != t.get() ) )

  return 0;
}
