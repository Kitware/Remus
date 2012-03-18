#include "client.h"

int main ()
{
  const int client_socket_num(5555);
  meshserver::Client c(client_socket_num);
  bool valid = c.execute();
  return valid ? 0 : 1;
}
