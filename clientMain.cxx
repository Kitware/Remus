#include "client.h"
#include "Common/meshServerGlobals.h"

int main ()
{
  const std::string ip = meshserver::make_tcp_conn(
                           "127.0.0.1",meshserver::BROKER_CLIENT_PORT);
  meshserver::Client c(ip);
  bool valid = c.execute();
  return valid ? 0 : 1;
}
