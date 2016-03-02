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
#include <remus/common/SignalCatcher.h>
#include <remus/testing/Testing.h>

#include <map>

//initialize the static instance variable of the signal catcher
remus::common::SignalCatcher* remus::common::SignalCatcher::Instance = NULL;

namespace
{
struct SignalCatcherVerifier : private remus::common::SignalCatcher
{
  SignalCatcher::SignalType ExpectedSignal;

  std::map< SignalType , std::string > sig_names;


  void start()
  {
    //make a lookup of names, so we can see in the output that we
    //are testing all signals, and see which one fails
    sig_names[ABORT]  = "ABORT";
    sig_names[FLOATING_POINT_ERROR]  = "FLOATING_POINT_ERROR";
    sig_names[ILLEGAL_INSTRUCTION]  = "ILLEGAL_INSTRUCTION";
    sig_names[INERRUPT]  = "INERRUPT";
    sig_names[SEGFAULT]  = "SEGFAULT";
    sig_names[TERMINATE]  = "TERMINATE";

    this->StartCatchingSignals();
  }

  void stop()
  {
    this->StopCatchingSignals();
  }

  void ExpectedSignalToCatch( SignalCatcher::SignalType signal )
  {
    this->ExpectedSignal = signal;
  }

  void SignalCaught( SignalCatcher::SignalType signal )
  {
  //make sure that the signal we catch is the one we expect to catch
  std::cout << "testing signal " << sig_names[signal] << std::endl;
  REMUS_ASSERT( (this->ExpectedSignal == signal) );
  }
};

}


int UnitTestSignalCatcher(int, char *[])
{
  SignalCatcherVerifier verifier;
  verifier.start();


//==============================================================================
//  Test catching signals
//==============================================================================

  //lets verify that it catches every single signal
  verifier.ExpectedSignalToCatch( remus::common::SignalCatcher::ABORT );
  raise(SIGABRT);

  verifier.ExpectedSignalToCatch( remus::common::SignalCatcher::FLOATING_POINT_ERROR );
  raise(SIGFPE);

  verifier.ExpectedSignalToCatch( remus::common::SignalCatcher::ILLEGAL_INSTRUCTION );
  raise(SIGILL);

  verifier.ExpectedSignalToCatch( remus::common::SignalCatcher::INERRUPT );
  raise(SIGINT);

  verifier.ExpectedSignalToCatch( remus::common::SignalCatcher::SEGFAULT );
  raise(SIGSEGV);

  verifier.ExpectedSignalToCatch( remus::common::SignalCatcher::TERMINATE );
  raise(SIGTERM);

  //verify that all signals have been released by the signal catcher
  //by checking that they now match SIG_DFL
  verifier.stop();
  std::cout << "testing signal catching stopped" << std::endl;

  void (*prev_sig_func)(int);
  prev_sig_func = signal( SIGABRT, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGFPE,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGILL,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGINT,  SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGSEGV, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  prev_sig_func = signal( SIGTERM, SIG_IGN );
  REMUS_ASSERT( (prev_sig_func == SIG_DFL) );

  //if we have reached this line we have caught all signal properly
  return 0;
}
