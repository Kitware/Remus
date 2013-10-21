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

#include <remus/worker/ServerConnection.h>
#include <remus/testing/Testing.h>

#include <string>

namespace {

std::string make_endPoint(std::string host, int port)
{
  const std::string conn_type( zmq::to_string(zmq::proto::tcp()) );
  std::string temp( conn_type + host + ":" +
                                  boost::lexical_cast<std::string>(port) );
  return temp;
}


} //namespace


int UnitTestServerConnection(int, char *[])
{

  remus::worker::ServerConnection sc;
  REMUS_ASSERT(sc.endpoint().size() > 0);

  const std::string default_endpoint = make_endPoint("127.0.0.1",
                                                   remus::SERVER_WORKER_PORT);
  REMUS_ASSERT( (sc.endpoint() == default_endpoint) );


  remus::worker::ServerConnection test_string_sc(default_endpoint);
  remus::worker::ServerConnection test_string_sc2(
                                                make_endPoint("127.0.0.1",1));

  REMUS_ASSERT( (test_string_sc.endpoint() == default_endpoint) );
  REMUS_ASSERT( (test_string_sc2.endpoint() != default_endpoint) );


  remus::worker::ServerConnection test_full_sc("foo",82);

  REMUS_ASSERT( (test_full_sc.endpoint() == make_endPoint("foo",82)) );
  REMUS_ASSERT( (test_full_sc.endpoint() != default_endpoint) );


  return 0;
}
