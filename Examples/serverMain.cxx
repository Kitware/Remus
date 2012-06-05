/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>
int main ()
{
  remus::server::WorkerFactory factory;
  factory.setMaxWorkerCount(4);

  remus::server::Server b(factory);

  bool valid = b.startBrokering();
  return valid ? 0 : 1;
}
