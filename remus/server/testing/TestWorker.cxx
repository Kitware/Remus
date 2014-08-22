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

#include <iostream>

#include <remus/common/SleepFor.h>
#include <remus/testing/Testing.h>

int main(int argc, char** argv)
{
  if(argc == 2)
    {
    std::string prog_type(argv[1]);
    if( prog_type.find("SLEEP_AND_EXIT") == 0)
      {
      remus::common::SleepForMillisec(1000);
      }
    else
      {
      return 1;
      }
    }
  return 0;
}
