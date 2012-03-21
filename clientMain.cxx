#include "client.h"

int main ()
{
  meshserver::Client c;
  bool valid = c.execute();
  return valid ? 0 : 1;
}
