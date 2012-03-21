/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __worker_h
#define __worker_h

#include "Common/meshServerGlobals.h"
#include "Common/mdcliapi.h"

namespace meshserver
{
class Client : public meshserver::mdp::client
{
public:

Client(const std::string& brokerIP):
  meshserver::mdp::client(brokerIP,true)
{

}

bool execute()
{
  for(int i=0; i < 100; ++i)
    {
    //send as a message the integer whose factorial we want to compute
//    std::stringstream buff;
//    buff << (100 + i * 4);
    zmsg* msg = new zmsg("Test");
    this->send(meshserver::MESH2D,msg);
    }

  for(int i=0; i < 100; ++i)
    {
    zmsg* reply = this->recv();
    if(reply)
      {
      std::cout << "Got a reply" << std::endl;
      delete reply;
      }
    else
      {
      break;
      }
    }
}

};
}


#endif
