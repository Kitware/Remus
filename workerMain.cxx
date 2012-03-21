#include "worker.h"

int main ()
{
  const std::string ip = meshserver::make_tcp_conn(
                           "127.0.0.1",meshserver::BROKER_PORT);

  meshserver::Worker w(ip);
  bool valid = w.execute();
  return valid ? 0 : 1;
}
