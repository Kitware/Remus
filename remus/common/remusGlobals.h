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

#ifndef __remus_common_RemusInfo_h
#define __remus_common_RemusInfo_h

#include <string>
#include <sstream>

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
// Severice Type macros.
#define ServiceTypeMacros() \
     ServiceTypeMacro(INVALID_SERVICE, 0,"INVALID"), \
     ServiceTypeMacro(MAKE_MESH, 1, "MAKE MESH"), \
     ServiceTypeMacro(MESH_STATUS, 2, "MESH STATUS"), \
     ServiceTypeMacro(CAN_MESH, 3, "CAN MESH"), \
     ServiceTypeMacro(RETRIEVE_MESH, 4, "RETRIEVE MESH"), \
     ServiceTypeMacro(HEARTBEAT, 5, "HEARTBEAT"), \
     ServiceTypeMacro(TERMINATE_JOB, 6, "TERMINATE JOB"), \
     ServiceTypeMacro(TERMINATE_WORKER, 7, "TERMINATE WORKER")

//------------------------------------------------------------------------------
enum SERVICE_TYPE
{
#define ServiceTypeMacro(ID,NUM,NAME) ID = NUM
   ServiceTypeMacros()
#undef ServiceTypeMacro
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
  static const char *serv_types[] = {
#define ServiceTypeMacro(ID,NUM,NAME) NAME
    ServiceTypeMacros()
#undef ServiceTypeMacro
  };
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
inline std::string extractString(std::stringstream& buffer, int size)
{
  if(buffer.peek()=='\n')
    {
    buffer.get();
    }

  std::string msg(size,char());
  char* raw = const_cast<char*>(msg.c_str());
  buffer.rdbuf()->sgetn(raw,size);
  return msg;
}

//------------------------------------------------------------------------------
inline void writeString(std::stringstream& buffer, const std::string str)
{
  buffer.rdbuf()->sputn(str.c_str(),str.length());
  buffer << std::endl;
}
}

}

#endif // __remus_common_RemusInfo_h
