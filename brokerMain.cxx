#include "broker.h"

int main ()
{
  meshserver::Broker b;
  b.start_brokering();
  return 1;
}
