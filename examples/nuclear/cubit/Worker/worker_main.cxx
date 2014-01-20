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
#include <cstdio>
#include <remus/worker/ServerConnection.h>
#include "worker.h"

int main (int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
    }
  worker w(connection );
  w.meshJob();
  return 0;
}
