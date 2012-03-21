#include "worker.h"

int main ()
{
  meshserver::Worker w;
  bool valid = w.execute();
  return valid ? 0 : 1;
}
