/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __meshserver_response_h
#define __meshserver_response_h

#include "stdint.h"
#include <zmq.hpp>
#include "meshServerGlobals.h"

namespace meshserver{
class JobResponse
{
  JobResponse();
  ~JobResponse();

  void markInValid(){ Valid = false; }

  //Clears any existing data message, and reconstructs
  //the new message we will use when we call send
  template<typename T>
  void setData(const T& t);

  bool send(zmq::socket_t& socket) const;

  private:
    bool Valid;
    zmq::message_t* DataMessage;
};

//------------------------------------------------------------------------------
JobResponse::JobResponse():
  Valid(true),
  DataMessage(NULL)
  {
  }

//------------------------------------------------------------------------------
template<typename T>
void JobResponse::setData(const T& t)
  {

  }

//------------------------------------------------------------------------------
bool JobResponse::send(zmq::socket_t& socket)
  {

  }

#endif // __meshserver_response_h
