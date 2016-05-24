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

#include <remus/common/SignalCatcher.h>

remus::common::SignalCatcher* remus::common::SignalCatcher::Instance = NULL;

namespace remus{
namespace common{

//------------------------------------------------------------------------------
SignalCatcher::~SignalCatcher()
{

  if( remus::common::SignalCatcher::Instance == this)
  {
    this->StopCatchingSignals();
  }
}

//------------------------------------------------------------------------------
void SignalCatcher::StartCatchingSignals()
{
  remus::common::SignalCatcher::Instance = this;

  //watch all signals that could cause the program to abnormally terminate
  signal( SIGABRT, remus::common::SignalCatcher::SIGCallback );
  signal( SIGINT,  remus::common::SignalCatcher::SIGCallback );
  signal( SIGTERM, remus::common::SignalCatcher::SIGCallback );

}

//------------------------------------------------------------------------------
void SignalCatcher::StopCatchingSignals()
{
  //stop watching all signals that could cause the program to
  //abnormally terminate
  signal( SIGABRT, SIG_DFL );
  signal( SIGINT,  SIG_DFL );
  signal( SIGTERM, SIG_DFL );

  remus::common::SignalCatcher::Instance = NULL;
}

//------------------------------------------------------------------------------
void SignalCatcher::SignalCaught( SignalCatcher::SignalType )
{

}

//------------------------------------------------------------------------------
void SignalCatcher::SIGCallback( int sig )
{
  remus::common::SignalCatcher::Instance->SignalCaught( SignalCatcher::SignalType(sig) );
}


}
}
