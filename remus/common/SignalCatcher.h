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

#ifndef __remus_common_SignalCatcher_h
#define __remus_common_SignalCatcher_h

#include <csignal>

#include <remus/common/CommonExports.h>

namespace remus{
namespace common{

// \brief SignalCatcher catch C abnormal termination signals
//
// This class gives the ability to catch unexpected signals like
// Seg fault, divide by zero, abort, etc. What we do is have this
// base class hook in and be called by the operating system signals
// which we than wrap and forward to virtual methods that classes that
// have derived from will implement.
//
// Since this class is hoisting C Signals you can only have a signal instance
// of this class in existence per process ever.
//
class REMUSCOMMON_EXPORT SignalCatcher
{
  static SignalCatcher* Instance;
public:
  enum SignalType
  {
    ABORT=SIGABRT,
    FLOATING_POINT_ERROR=SIGFPE,
    ILLEGAL_INSTRUCTION=SIGILL,
    INERRUPT=SIGINT,
    SEGFAULT=SIGSEGV,
    TERMINATE=SIGTERM
  };

protected:

  //Call this method to start watching csignals
  //
  void StartCatchingSignals()
  {
    remus::common::SignalCatcher::Instance = this;

    //watch all signals that could cause the program to abnormally terminate
    signal( SIGABRT, remus::common::SignalCatcher::SIGCallback );
    signal( SIGFPE,  remus::common::SignalCatcher::SIGCallback );
    signal( SIGILL,  remus::common::SignalCatcher::SIGCallback );
    signal( SIGINT,  remus::common::SignalCatcher::SIGCallback );
    signal( SIGSEGV, remus::common::SignalCatcher::SIGCallback );
    signal( SIGTERM, remus::common::SignalCatcher::SIGCallback );

  }

  void StopCatchingSignals()
  {
    remus::common::SignalCatcher::Instance = this;

    //stop watching all signals that could cause the program to
    //abnormally terminate
    signal( SIGABRT, SIG_DFL );
    signal( SIGFPE,  SIG_DFL );
    signal( SIGILL,  SIG_DFL );
    signal( SIGINT,  SIG_DFL );
    signal( SIGSEGV, SIG_DFL );
    signal( SIGTERM, SIG_DFL );
  }

  //method to override to pickup what signal was thrown
  //
  virtual void SignalCaught( SignalCatcher::SignalType )
  {

  }


private:

  //catches all signals we are watching and converts the signal type
  //to SignalCatcher::SignalType
  //
  static void SIGCallback( int sig )
  { remus::common::SignalCatcher::Instance->SignalCaught( SignalCatcher::SignalType(sig) ); }

};

}
}
#endif
