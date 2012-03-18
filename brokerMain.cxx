#include "broker.h"

int main ()
{
  const int client_socket_num(5555);
  const int worker_socket_num(5556);
  meshserver::Broker b(client_socket_num, worker_socket_num);
  bool valid = b.execute();
  return valid ? 0 : 1;
}
