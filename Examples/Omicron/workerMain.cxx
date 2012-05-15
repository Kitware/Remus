/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"

int main (int argc, char* argv[])
{
  meshserver::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = meshserver::worker::ServerConnection(std::string(argv[1]));
    }
  OmicronWorker worker( connection );
  worker.meshJob();
  return 1;
}
