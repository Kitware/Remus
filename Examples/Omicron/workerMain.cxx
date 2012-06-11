/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"

int main (int argc, char* argv[])
{
  //create a default server connection object
  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //if we are passed a custom end point to connect to on the command line
    //use that over the default server connection options
    connection = remus::worker::ServerConnection(std::string(argv[1]));
    }

  //create a 3d omicron mesher
  OmicronWorker worker(remus::MESH3D, connection );
  worker.setExecutableName("model");
  worker.meshJob();
  return 1;
}
