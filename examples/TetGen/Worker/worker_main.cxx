#include <cstdio>
#include <remus/worker/ServerConnection.h>
#include "TetgenWorker.h"

int main (int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
    }
  //the triangle worker is hard-coded to only handle 3D meshes output, and
  //input of PIECWISE_LINEAR_COMPLEX
  TetGenWorker worker( connection );
  worker.meshJob();
  return 0;
}