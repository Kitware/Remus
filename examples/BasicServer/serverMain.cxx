/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

#include <iostream>
int main (int argc, char* argv[])
{
  remus::server::ServerPorts ports;
  //check if we have a hostname to bind too.
  if(argc>=2)
    {
    std::string hostname(argv[1]);
    ports = remus::server::ServerPorts(hostname,remus::SERVER_CLIENT_PORT,
                                       hostname,remus::SERVER_WORKER_PORT);
    }

  //create a custom worker factory that creates children processes
  //we cap it at having only 3 children at any time
  boost::shared_ptr<remus::server::WorkerFactory> factory(
                    new remus::server::WorkerFactory() );
  factory->setMaxWorkerCount(3);

  //create a default server with the factory
  remus::server::Server b(ports,factory);

  //start accepting connections for clients and workers
  bool valid = b.startBrokeringWithoutSignalHandling();
  std::cout << "Brokering started, waiting for finishing" << std::endl;
  b.waitForBrokeringToFinish();
  return valid ? 0 : 1;
}
