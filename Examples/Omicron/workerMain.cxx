/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"

int main (int argc, char* argv[])
{
  //first argument is the server you want to connect too
  meshserver::worker::ServerConnection connection(argc,argv);
  OmicronWorker worker( connection );
  worker.meshJob();
  return 1;
}
