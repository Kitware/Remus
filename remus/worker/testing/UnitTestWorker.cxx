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
#include <remus/worker/Worker.h>
#include <remus/testing/Testing.h>

#include <remus/proto/zmq.hpp>

#include <boost/scoped_ptr.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <boost/thread.hpp>
#pragma GCC diagnostic pop


#include <string>

namespace {

zmq::socketInfo<zmq::proto::tcp> make_tcp_socket(std::string host, int port)
{
  return zmq::socketInfo<zmq::proto::tcp>(host,port);
}

zmq::socketInfo<zmq::proto::ipc> make_ipc_socket(std::string host)
{
  return zmq::socketInfo<zmq::proto::ipc>(host);
}

zmq::socketInfo<zmq::proto::inproc> make_inproc_socket(std::string host)
{
  return zmq::socketInfo<zmq::proto::inproc>(host);
}

class fake_server
{
public:
  fake_server(remus::worker::ServerConnection& conn):
    WorkerComm((*conn.context()),ZMQ_ROUTER),
    PollingThread( new boost::thread() ),
    ContinuePolling(true)
  {
    this->WorkerComm.bind( conn.endpoint().c_str() );
    //start up our thread
    boost::scoped_ptr<boost::thread> pollingThread(
                             new boost::thread( &fake_server::poll, this) );
    //transfer ownership of the polling thread to our scoped_ptr
    this->PollingThread.swap(pollingThread);
  }

  ~fake_server()
  {
    this->ContinuePolling = false;
    this->PollingThread->join();
  }

private:
  void poll()
  {
    zmq::pollitem_t item  = { this->WorkerComm,  0, ZMQ_POLLIN, 0 };
    while( this->ContinuePolling )
      {
      zmq::poll(&item,1,250);
      }
  }

  zmq::socket_t WorkerComm;
  boost::scoped_ptr<boost::thread> PollingThread;
  bool ContinuePolling;
};

void verify_server_connection()
{
  using namespace remus::meshtypes;
  const remus::common::MeshIOType mtype =
                          remus::common::make_MeshIOType(Model(),Model());

  remus::worker::ServerConnection default_sc;

  fake_server fake_def_server(default_sc); //start up server to talk to worker
  remus::worker::Worker default_worker(mtype,default_sc);

  const remus::worker::ServerConnection& sc = default_worker.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::SERVER_WORKER_PORT);
  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );
  REMUS_ASSERT( (sc.endpoint() ==
         make_tcp_socket("127.0.0.1",remus::SERVER_WORKER_PORT).endpoint()) );


  remus::worker::ServerConnection ipc_conn =
        remus::worker::make_ServerConnection("ipc://foo_ipc");
  fake_server ipc_server(ipc_conn);
  remus::worker::Worker ipc_worker(mtype,ipc_conn);

  REMUS_ASSERT( (ipc_worker.connection().endpoint() ==
                 make_ipc_socket("foo_ipc").endpoint()) );

  remus::worker::ServerConnection inproc_conn =
         remus::worker::make_ServerConnection("inproc://foo_inproc");

  fake_server inproc_server(inproc_conn);
  remus::worker::Worker inproc_worker(mtype,inproc_conn);


  REMUS_ASSERT( (inproc_worker.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );

  //share a connection between two workers that share the same context and
  //channel. This shows that you can have multiple works sharing the same
  //inproc or ipc context.
  remus::worker::ServerConnection conn2 =
    remus::worker::make_ServerConnection(inproc_worker.connection().endpoint());
  conn2.context(inproc_conn.context());


  remus::worker::Worker inproc_worker2(mtype,conn2);
  REMUS_ASSERT( (inproc_worker2.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );
  REMUS_ASSERT( (inproc_worker.connection().context() ==
                 inproc_worker2.connection().context() ) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (ipc_worker.connection().isLocalEndpoint()==true) );
  REMUS_ASSERT( (inproc_worker.connection().isLocalEndpoint()==true) );
}

} //namespace


int UnitTestWorker(int, char *[])
{
  verify_server_connection();
  return 0;
}
