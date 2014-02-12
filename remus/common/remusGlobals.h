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

#ifndef remus_common_RemusGlobals_h
#define remus_common_RemusGlobals_h

#include <string>
#include <sstream>
#include <vector>

//Define global information that the mesh server needs
namespace remus {

//SERVER_CLIENT_PORT is the port that clients connect on, to
//submit and query about jobs
static const int SERVER_CLIENT_PORT = 50505;

//SERVER_WORKER_PORT is used by workers to fetch jobs,
//and send back the status and result of the job
//Workers also use the SERVER_WORKER_PORT for heartbeating
static const int SERVER_WORKER_PORT = 50510;

static const int HEARTBEAT_INTERVAL_IN_SEC = 5;
static const int HEARTBEAT_INTERVAL = 1000 * HEARTBEAT_INTERVAL_IN_SEC;

static const std::string INVALID_MSG = "INVALID_MSG";

//------------------------------------------------------------------------------
enum SERVICE_TYPE
{
  INVALID_SERVICE = 0,
  MAKE_MESH = 1,
  MESH_STATUS = 2,
  CAN_MESH = 3,
  RETRIEVE_MESH = 4,
  HEARTBEAT = 5,
  //we should split up the TERMINATE_JOB_AND_WORKER into really two calls
  //the first would be stop_job which would only stop the current job
  //it might terminate the worker if needed, but no mandatory. The
  //second would be to terminate a worker and all jobs it contains
  TERMINATE_JOB = 6,
  TERMINATE_WORKER = 7,
  MESH_REQUIREMENTS = 8
};

//------------------------------------------------------------------------------
enum STATUS_TYPE
{
  INVALID_STATUS = 0,
  QUEUED = 1,
  IN_PROGRESS = 2,
  FINISHED = 3,
  FAILED = 4,
  EXPIRED = 5
};


//------------------------------------------------------------------------------
namespace common
  {
  //a mapping of enum types to char*
  static const char *serv_types[] = { "INVALID", "MAKE MESH", "MESH STATUS", "CAN MESH", "RETRIEVE MESH", "HEARTBEAT", "TERMINATE JOB", "TERMINATE WORKER" };
  static const char *stat_types[] = { "INVALID", "QUEUED", "IN PROGRESS", "FINISHED", "FAILED","EXPIRED" };
  }

//------------------------------------------------------------------------------
inline std::string to_string(remus::SERVICE_TYPE t)
{
  return std::string(remus::common::serv_types[(int)t]);
}

//------------------------------------------------------------------------------
inline std::string to_string(remus::STATUS_TYPE t)
{
  return std::string(remus::common::stat_types[(int)t]);
}

//------------------------------------------------------------------------------
inline remus::SERVICE_TYPE to_serviceType(const std::string& t)
{
  for(int i=1; i <=6; i++)
    {
    remus::SERVICE_TYPE mt=static_cast<remus::SERVICE_TYPE>(i);
    if (remus::to_string(mt) == t)
      {
      return mt;
      }
    }
  return remus::INVALID_SERVICE;
}

namespace internal
{
//------------------------------------------------------------------------------
template<typename BufferType>
inline void extractVector(BufferType& buffer, std::vector<char>& msg)
{
  if(buffer.peek()=='\n')
    {
    buffer.get();
    }
  buffer.rdbuf()->sgetn(&msg[0],msg.size());
}

//------------------------------------------------------------------------------
template<typename BufferType>
inline std::string extractString(BufferType& buffer, int size)
{
  std::vector<char> msg(size);
  extractVector(buffer,msg);
  return std::string(&msg[0],size);
}


//------------------------------------------------------------------------------
template<typename BufferType>
inline void writeString(BufferType& buffer, const std::string& str)
{
  buffer.rdbuf()->sputn(str.c_str(),str.length());
  buffer << std::endl;
}


//------------------------------------------------------------------------------
template<typename BufferType>
inline void writeString(BufferType& buffer, const char * str, std::size_t size)
{
  buffer.rdbuf()->sputn(str,size);
  buffer << std::endl;
}

}

}

#endif // remus_common_RemusGlobals_h
