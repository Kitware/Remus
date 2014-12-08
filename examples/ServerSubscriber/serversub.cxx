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

  //state we will take only requested messages
  if (argc < 2)
    {
    //take all messages when no arguments are passed to main().
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
    }

  // Examples you can pass on the command line:
  //   worker:REGISTERED
  //   worker:ASKING_FOR_JOB
  //   job:ASSIGNED_TO_WORKER

  for (int arg = 1; arg < argc; ++arg)
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
      argv[arg], strlen(argv[arg]));

  // Always listen for "stop"
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
  std::cout << "value:" << (rendered ? rendered : "(empty)") << std::endl;

  free(rendered);
  cJSON_Delete(root);
  }

  return 0;
}
