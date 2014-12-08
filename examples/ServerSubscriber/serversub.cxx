/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <remus/client/Client.h>

#include <remus/proto/zmqHelper.h>
#include <remus/proto/zmq.hpp>

#include <vector>
#include <iostream>

#include "cJSON.h"

int main(int argc, char* argv[])
{
  //manually connect to the server pub socket
  //bind the sockets manually and verify that the binding works.
  zmq::socketInfo<zmq::proto::tcp> default_sub("127.0.0.1",
                                               remus::SERVER_SUB_PORT);
  zmq::context_t context(1);
  zmq::socket_t subscriber(context,ZMQ_SUB);

  //bind the socket
  zmq::connectToAddress(subscriber,default_sub);

  //state we will take only job messages
  std::string workerSub = "worker:REGISTERED";
  std::string workerSub2 = "worker:ASKING_FOR_JOB";
  std::string jobSub = "job:ASSIGNED_TO_WORKER";
  zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, workerSub.c_str(), workerSub.size());
  zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, workerSub2.c_str(), workerSub2.size());
  zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, jobSub.c_str(), jobSub.size());


  std::string stopSub = "stop";
  zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, stopSub.c_str(), stopSub.size());


  //dump all messages
  const std::string termination_message = "END";
  zmq::message_t key;
  zmq::message_t data;
  while (true)
  {
  zmq::recv_harder(subscriber, &key);
  zmq::recv_harder(subscriber, &data);

  std::string keyStr( reinterpret_cast<char*>(key.data()), key.size() );

  if(keyStr == stopSub)
    {
    return 0;
    }

  cJSON *root = cJSON_Parse( reinterpret_cast<char*>(data.data()) );
  char *rendered = cJSON_Print(root);
  std::cout << "key: " << keyStr << std::endl;
  std::cout << "value:" << rendered << std::endl;

  free(rendered);
  cJSON_Delete(root);
  }

  return 0;
}
