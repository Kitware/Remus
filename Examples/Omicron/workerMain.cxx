/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "OmicronWorker.h"

int main (int argc, char* argv[])
{
  //first argument is the server you want to connect too
  OmicronWorker *worker;
  if (argc == 2)
    {
    std::string serverEndpoint(argv[1]);
    worker =  new OmicronWorker(serverEndpoint);
    }
  else
    {
    worker = new OmicronWorker();
    }
  worker->meshJob();
  delete worker;
  return 1;
}
