/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <meshserver/server/Server.h>
#include <meshserver/server/WorkerFactory.h>
int main ()
{
  meshserver::server::WorkerFactory factory;
  factory.setMaxWorkerCount(4);

  meshserver::server::Server b(factory);

  bool valid = b.startServering();
  return valid ? 0 : 1;
}
