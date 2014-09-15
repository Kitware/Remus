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

#include <remus/common/SleepFor.h>
#include <remus/worker/ServerConnection.h>
#include <remus/worker/Worker.h>
#include <remus/testing/Testing.h>

#include <remus/proto/zmqHelper.h>

#include <boost/scoped_ptr.hpp>

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/thread.hpp>
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif


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
  template<typename ProtoType>
  fake_server(zmq::socketInfo<ProtoType>& conn,
               boost::shared_ptr<zmq::context_t> context):
    WorkerComm((*context),ZMQ_ROUTER),
    PollingThread( new boost::thread() ),
    ContinuePolling(true)
  {
    zmq::bindToAddress(this->WorkerComm, conn);

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

void verify_server_connection_tcpip()
{
  using namespace remus::meshtypes;
  const remus::common::MeshIOType mtype =
                          remus::common::make_MeshIOType(Model(),Model());

  zmq::socketInfo<zmq::proto::tcp> local_socket("127.0.0.1",
                                        remus::SERVER_WORKER_PORT+101);
  remus::worker::ServerConnection tcp_ip_conn(local_socket);

  //start up server to talk to worker
  fake_server fake_def_server(local_socket, tcp_ip_conn.context());
  remus::worker::Worker default_worker(mtype,tcp_ip_conn);

  const remus::worker::ServerConnection& sc = default_worker.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  REMUS_ASSERT( (sc.endpoint() == local_socket.endpoint()) );
  REMUS_ASSERT( (sc.endpoint() ==
       make_tcp_socket("127.0.0.1",remus::SERVER_WORKER_PORT+101).endpoint()) );

  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
}

void verify_server_connection_inproc()
{
  using namespace remus::meshtypes;
  const remus::common::MeshIOType mtype =
                          remus::common::make_MeshIOType(Model(),Model());

  zmq::socketInfo<zmq::proto::inproc> inproc_info("foo_inproc");
  remus::worker::ServerConnection inproc_conn(inproc_info);

  fake_server inproc_server(inproc_info, inproc_conn.context());
  remus::worker::Worker inproc_worker(mtype,inproc_conn);

  REMUS_ASSERT( (inproc_worker.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );

  //share a connection between two workers that share the same context and
  //channel. This shows that you can have multiple works sharing the same
  //inproc context.
  remus::worker::ServerConnection conn2 =
    remus::worker::make_ServerConnection(inproc_worker.connection().endpoint());
  conn2.context(inproc_conn.context());


  remus::worker::Worker inproc_worker2(mtype,conn2);
  REMUS_ASSERT( (inproc_worker2.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );
  REMUS_ASSERT( (inproc_worker.connection().context() ==
                 inproc_worker2.connection().context() ) );

  REMUS_ASSERT( (inproc_worker.connection().isLocalEndpoint()==true) );
}

void verify_server_connection_ipc()
{
  using namespace remus::meshtypes;
  const remus::common::MeshIOType mtype =
                          remus::common::make_MeshIOType(Model(),Model());

  zmq::socketInfo<zmq::proto::ipc> ipc_info("foo_ipc");
  remus::worker::ServerConnection ipc_conn(ipc_info);

  fake_server ipc_server(ipc_info,ipc_conn.context());
  remus::worker::Worker ipc_worker(mtype,ipc_conn);

  REMUS_ASSERT( (ipc_worker.connection().endpoint() ==
                 make_ipc_socket("foo_ipc").endpoint()) );
  REMUS_ASSERT( (ipc_worker.connection().isLocalEndpoint()==true) );

  //share a connection between two workers that share the same context and
  //channel. This shows that you can have multiple works sharing the same
  //ipc context.
  remus::worker::ServerConnection conn2 =
    remus::worker::make_ServerConnection(ipc_worker.connection().endpoint());
  conn2.context(ipc_conn.context());
  remus::worker::Worker ipc_worker2(mtype,conn2);

  REMUS_ASSERT( (ipc_worker2.connection().endpoint() ==
                 make_ipc_socket("foo_ipc").endpoint()) );
  REMUS_ASSERT( (ipc_worker.connection().context() ==
                 ipc_worker2.connection().context() ) );

}

} //namespace


int UnitTestWorker(int, char *[])
{
  verify_server_connection_tcpip();
  verify_server_connection_inproc();
#ifndef _WIN32
  verify_server_connection_ipc();
#endif
  return 0;
}
