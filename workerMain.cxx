#include "worker.h"

int main ()
{

  const int worker_socket_num(5556);
  meshserver::Worker w(worker_socket_num);
  std::cout << "about to execute" << std::endl;
  bool valid = w.execute();
  return valid ? 0 : 1;
}
