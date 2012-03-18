#include "meshWorker.h"

int main ()
{
  const int worker_socket_num(5556);
  meshserver::MeshWorker w(worker_socket_num);
  bool valid = w.execute();
  return valid ? 0 : 1;
}
