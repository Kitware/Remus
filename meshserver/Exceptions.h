#ifndef __meshserver_Exceptions_h
#define __meshserver_Exceptions_h

#include <zmq.hpp>
#include <string>

namespace meshserver{

class ZmqError
{
public:
  ZmqError(const char* zmsg):
    ZmqMsg(zmsg)
    {
    }

  virtual const char *what () const throw ()
  {
    return (std::string("Zmq error is: ") + ZmqMsg).c_str();
  }


private:
  const std::string ZmqMsg;
};

}

#endif // __meshserver_internal_Exceptions_h
