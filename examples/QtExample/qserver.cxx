/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "qserver.h"

#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

#include <boost/make_shared.hpp>
#include <iostream>

qserver::qserver():
  QObject(0),
  Server( new remus::server::Server(
                boost::make_shared<remus::server::WorkerFactory>( "fofof" ) ) )
{
  //set factory to an extension that is never used so that
  //all the workers come from the controls class threads
  this->Server->startBrokeringWithoutSignalHandling();
  std::cout << "server is launched" << std::endl;
  emit started();
}

qserver::~qserver()
{
  this->Server->stopBrokering();
  emit stopped();
}
