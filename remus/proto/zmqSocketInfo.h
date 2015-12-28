//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef remus_proto_zmqSocketInfo_h
#define remus_proto_zmqSocketInfo_h

#include <remus/common/CompilerInformation.h>
#include <remus/proto/zmqTraits.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/lexical_cast.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

//inject some basic zero MQ helper functions into the namespace
namespace zmq
{

//holds the information needed to construct a zmq socket connection
//from zmq documentation:
//endpoint argument is a string consisting of two parts as follows:
//transport ://address. The transport part specifies the underlying transport
//protocol to use. The meaning of the address part is specific to the underlying
//transport protocol selected.
//for the socket info class we are going to separate out the port of the tcp

//templated based on the transport types
template<typename _Proto>
struct socketInfo
{
  typedef _Proto Protocall;

  socketInfo():Host(){}
  explicit socketInfo(const std::string& hostName):
    Host(hostName)
  {
  }

  std::string endpoint() const {return  zmq::proto::scheme_and_separator(Protocall()) + Host;}
  const std::string& host() const{ return Host; }
  std::string scheme() const { return zmq::proto::scheme_name(Protocall());}
  int port() const { return -1; }
private:
  std::string Host;
};

template<> struct socketInfo<zmq::proto::tcp>
{
  typedef zmq::proto::tcp Protocall;

  socketInfo():Host(),Port(-1){}
  explicit socketInfo(const std::string& hostName, int portNum):
    Host(hostName),
    Port(portNum)
  {
  }

  std::string endpoint() const
    {
    return  zmq::proto::scheme_and_separator(Protocall()) + Host + ":" + boost::lexical_cast<std::string>(Port);
    }

  const std::string& host() const{ return Host; }
  std::string scheme() const { return zmq::proto::scheme_name(Protocall());}

  int port() const { return Port; }

  void setPort(int p){ Port=p; }

private:
  std::string Host;
  int Port;
};


//returns trues if the socket points to a local server. A tcp-ip connection
//can be local or remote. We are going to only state that 127.0.0.1 is local
//host, this is because zmq for connect requires ipv4 numeric address. It
//is possible that a specific ip address is also local, but really I don't
//want to do the work to find all the possible ip addresses a machine has
inline bool isLocalEndpoint(const zmq::socketInfo<zmq::proto::tcp>& info)
{
  return info.host() == "127.0.0.1";
}

//returns trues if the socket points to a local server. A inter-thread(inproc)
//connection is only local so this is automatically true
inline bool isLocalEndpoint(zmq::socketInfo<zmq::proto::inproc> )
{ return true; }

//returns trues if the socket points to a local server. A inter-process(ipc)
//connection is only local so this is automatically true
inline bool isLocalEndpoint(zmq::socketInfo<zmq::proto::ipc> )
{ return true; }


}

#endif
