#include "broker.h"

int main ()
{
  meshserver::Broker b;
  bool valid = b.startBrokering();
  return valid ? 0 : 1;
}
