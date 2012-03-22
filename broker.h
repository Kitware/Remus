/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __server_h
#define __server_h

#include <iostream>
#include <sstream>

#include "Common/mdbrkapi.h"
#include "Common/meshServerGlobals.h"

namespace meshserver
{
class Broker : public meshserver::mdp::broker
{
public:Broker() : meshserver::mdp::broker(true)
  {

  }
};
}


#endif
