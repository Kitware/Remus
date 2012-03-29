/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/Broker.h>

int main ()
{
  meshserver::broker::Broker b;
  bool valid = b.startBrokering();
  return valid ? 0 : 1;
}
