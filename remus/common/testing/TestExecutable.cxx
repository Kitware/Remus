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
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

int main(int argc, char** argv)
{
  //never ending application that needs to be killed
  //and just spews values
  int pollingType = -1;
  int exitNormally = -1;
  if(argc == 2)
    {
    std::string prog_type(argv[1]);
    if(prog_type.find("NO_POLL") == 0)
      pollingType = 1;
    else if( prog_type.find("POLL") == 0)
      pollingType = 2;
    else if( prog_type.find("EXIT_NORMALLY") == 0)
      exitNormally = 1;
    }

  //determine our behavior
  if(exitNormally == 1)
    {
#ifdef _WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
    }
  else if(pollingType == 1)
    {
    while(true){}
    }
  else if(pollingType == 2)
    {
    while(true){  std::cout << "fake polling output" << std::endl; }
    }
  else
    {
    while(true){  std::cout << "default behavior" << std::endl; }
    }

  return 0;
}
