#include "broker.h"

int main ()
{
  meshserver::Broker b;
  bool valid = b.execute();
  return valid ? 0 : 1;
}
