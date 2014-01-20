#include <iostream>

#include "client.h"

int main (int argc, char* argv[])
{
  client c((argc >= 4)?std::string(argv[3]):std::string(""));
  if (argc < 3)
  {
    std::cerr << "Need to give a prefix" << std::endl;
  }
  std::string pref =argv[2];
  std::string exe = argv[1];
  AssygenInput ai( pref, exe );
  AssygenOutput r = c.getOutput(ai);
  std::cout << "Result: " << (std::string)(r) << std::endl;
  return 0;
}
