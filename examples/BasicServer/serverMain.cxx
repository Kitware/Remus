/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

#include <iostream>
int main ()
{
  //create a custom worker factory that creates children processes
  //we cap it at having only 3 children at any time
  remus::server::WorkerFactory factory;
  factory.setMaxWorkerCount(3);

  //create a default server with the factory
  remus::server::Server b(factory);

  //start accepting connections for clients and workers
  bool valid = b.startBrokering();
  std::cout << "Brokering started, waiting for finishing" << std::endl;
  b.waitForBrokeringToFinish();
  return valid ? 0 : 1;
}
