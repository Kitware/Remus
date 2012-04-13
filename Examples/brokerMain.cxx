/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/broker/Broker.h>
#include <meshserver/broker/WorkerFactory.h>
int main ()
{
  meshserver::broker::WorkerFactory factory;
  factory.setMaxWorkerCount(4);

  meshserver::broker::Broker b(factory);

  bool valid = b.startBrokering();
  return valid ? 0 : 1;
}
