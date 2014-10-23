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

#include <fstream>
#include <iostream>

#include <remus/common/SleepFor.h>
#include <remus/testing/Testing.h>

#include <stdlib.h>

#include <time.h>
#include <sys/time.h>

#define REMUS_WORKER_ENV_TEST "REMUS_WORKER_ENV_TEST"

int main(int argc, char** argv)
{
  timeval tm;
  gettimeofday(&tm, NULL);
  std::ofstream tlog("/tmp/tlog", std::ios_base::app);
  char* tbuf = ctime(&tm.tv_sec);
  tbuf[24] = '\0';
  tlog << tbuf << ": ";
  for (int i = 0; i < argc; ++i)
    tlog << " " << argv[i];
  tlog << "\n";

  if(argc == 2)
    {
    std::string prog_type(argv[1]);
    if( prog_type.find("SLEEP_AND_EXIT") == 0)
      {
      remus::common::SleepForMillisec(1000);
      tlog << "  sleep and exit 0\n";
      }
    else
      {
      tlog << "  " << argv[1] << " 1\n";
      return 1;
      }
    return 0;
    }
  // This worker should get run once with both an
  // environment variable and 2 additional command
  // line arguments. If we detect both, the test is
  // a success. Otherwise we should fail.
  //
  // NB: In worker files, an "@SELF@" in a command
  //     line argument should get replaced with the
  //     path to the worker file at run time.
  //     The test below prevents regressions.
  else if (
    argc > 2 &&
    std::string(argv[1]) == "-argtest" &&
    std::string(argv[2]) != "@SELF@")
    {
    char* buf;
    std::string env;
#if !defined(_WIN32) || defined(__CYGWIN__)
    buf = getenv(REMUS_WORKER_ENV_TEST);
    if (buf && buf[0])
      env = buf;
#else
    const bool valid =
      (_dupenv_s(&buf, NULL, REMUS_WORKER_ENV_TEST) == 0) &&
      (buf != NULL);
    if (valid)
      env = buf;
#endif
    tlog << "  arg env test " << (env == "TRUE" ? 0 : 1) << "\n";
    if (env == "TRUE")
      return 0;
    }
  tlog << "  return 1\n";
  return 1;
}
