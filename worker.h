/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __worker_h
#define __worker_h

#include "Common/meshServerGlobals.h"
#include "Common/mdwrkapi.h"

#include <sstream>
#include <iostream>

namespace meshserver
{

int find_factorial(int n)
{
  return (n==0)? 1 : n * find_factorial(n-1);
}

class Worker : public meshserver::mdp::worker
{
public:

Worker(const std::string brokerIP):
  meshserver::mdp::worker(brokerIP,meshserver::MESH2D,true)
  {
  }

bool execute()
{
  zmsg* reply = NULL;
  while(true)
    {
    zmsg* request = this->recv(reply);
    if(request==NULL)
      {
      break;
      }
    reply = request;
//    std::stringstream buffer(reply->body());
//    int factorial;
//    buffer >> factorial;
//    std::cout << "factorial " << factorial << " is: " << find_factorial(factorial) << std::endl;
    }
}

};
}


#endif
