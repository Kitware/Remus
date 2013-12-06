#include <cstdio>
#include <remus/worker/ServerConnection.h>
#include "TriangleWorker.h"

int main (int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
    }
  //the triangle worker is hard-coded to only hand 2D meshes output, and
  //input of RAW_EDGES
  TriangleWorker worker(connection );
  worker.meshJob();
  return 0;
}