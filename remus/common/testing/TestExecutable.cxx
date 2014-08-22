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

int main(int argc, char** argv)
{
  //never ending application that needs to be killed
  //and just spews values
  int mode = -1;
  if(argc == 2)
    {
    std::string prog_type(argv[1]);
    if( prog_type.find("SLEEP_AND_EXIT") == 0)
      mode = 0;
    else if(prog_type.find("NO_OUTPUT") == 0)
      mode = 1;
    else if( prog_type.find("COUT_OUTPUT") == 0)
      mode = 2;
    else if( prog_type.find("CERR_OUTPUT") == 0)
      mode = 3;
    }

  //determine our behavior
  if(mode == 0)
    {
    remus::common::SleepForMillisec(500);
    }
  else if(mode >= 1)
    { //no ouput while polling
    while(true)
      {
      if(mode == 2)
        {
        std::cout << "standard channel output" << std::endl;
        }
      else if(mode == 3)
        {
        std::cerr << "error channel output" << std::endl;
        }
      }
    }
  return 0;
}
