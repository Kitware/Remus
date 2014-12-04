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

//Define global information that the mesh server needs
namespace remus {

//SERVER_CLIENT_PORT is the port that clients connect on, to
//submit and query about jobs
static const int SERVER_CLIENT_PORT = 50505;

//SERVER_WORKER_PORT is used by workers to fetch jobs,
//and send back the status and result of the job
//Workers also use the SERVER_WORKER_PORT for heart beating
static const int SERVER_WORKER_PORT = 50510;

//SERVER_SUB_PORT is the port the server publishes all forms of async
//notifications onto. Clients, Loggers, and Monitors can connect to this
//port to watch the status of jobs, see the health of the server, etc.
static const int SERVER_SUB_PORT = 50550;

static const std::string INVALID_MSG = "INVALID_MSG";

//------------------------------------------------------------------------------
// Severice Type macros.
#define ServiceTypeMacros() \
     ServiceTypeMacro(INVALID_SERVICE, 0,"INVALID"), \
     ServiceTypeMacro(SUPPORTED_IO_TYPES, 1, "SUPPORTED IO TYPES"), \
     ServiceTypeMacro(MESH_REQUIREMENTS_FOR_IO_TYPE, 2, "MESH REQUIREMENTS IO TYPE"), \
     ServiceTypeMacro(CAN_MESH_REQUIREMENTS, 3, "CAN MESH REQUIREMENTS"), \
     ServiceTypeMacro(CAN_MESH_IO_TYPE, 4, "CAN MESH IO TYPE"), \
     ServiceTypeMacro(MAKE_MESH, 5, "MAKE MESH"), \
     ServiceTypeMacro(MESH_STATUS, 6, "MESH STATUS"), \
     ServiceTypeMacro(RETRIEVE_RESULT, 7, "RETRIEVE RESULT"), \
     ServiceTypeMacro(HEARTBEAT, 8, "HEARTBEAT"), \
     ServiceTypeMacro(TERMINATE_JOB, 9, "TERMINATE JOB"), \
     ServiceTypeMacro(TERMINATE_WORKER, 10, "TERMINATE WORKER")


//------------------------------------------------------------------------------
enum SERVICE_TYPE
{
#define ServiceTypeMacro(ID,NUM,NAME) ID = NUM
   ServiceTypeMacros()
#undef ServiceTypeMacro
};

//------------------------------------------------------------------------------
// Status Type macros
#define StatusTypeMacros() \
     StatusTypeMacro(INVALID_STATUS, 0,"INVALID"), \
     StatusTypeMacro(QUEUED, 1, "QUEUED"), \
     StatusTypeMacro(IN_PROGRESS, 2, "IN PROGRESS"), \
     StatusTypeMacro(FINISHED, 3, "FINISHED"), \
     StatusTypeMacro(FAILED, 4, "FAILED"), \
     StatusTypeMacro(EXPIRED, 5, "EXPIRED")

//------------------------------------------------------------------------------
enum STATUS_TYPE
{
#define StatusTypeMacro(ID,NUM,NAME) ID = NUM
  StatusTypeMacros()
#undef StatusTypeMacro
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
  static const char *stat_types[] = {
#define StatusTypeMacro(ID,NUM,NAME) NAME
    StatusTypeMacros()
#undef StatusTypeMacro
  };
  }

//------------------------------------------------------------------------------
inline std::string to_string(remus::SERVICE_TYPE t)
{
  return std::string(remus::common::serv_types[(int)t]);
}

//------------------------------------------------------------------------------
inline remus::SERVICE_TYPE to_serviceType(const std::string& t)
{
  for(int i=1; i<=10; i++)
    {
    remus::SERVICE_TYPE mt=static_cast<remus::SERVICE_TYPE>(i);
    if (remus::to_string(mt) == t)
      {
      return mt;
      }
    }
  return remus::INVALID_SERVICE;
}

//------------------------------------------------------------------------------
inline std::string to_string(remus::STATUS_TYPE t)
{
  return std::string(remus::common::stat_types[(int)t]);
}

//------------------------------------------------------------------------------
inline remus::STATUS_TYPE to_statusType(const std::string& t)
{
  for(int i=1; i <=5; i++)
    {
    remus::STATUS_TYPE mt=static_cast<remus::STATUS_TYPE>(i);
    if (remus::to_string(mt) == t)
      {
      return mt;
      }
    }
  return remus::INVALID_STATUS;
}

}

//Now define some global windows suppressions
#if defined(_MSC_VER) // Visual studio
# pragma warning ( disable : 4251 )  //missing DLL-interface
# pragma warning ( disable : 4514 ) //unreferenced inline function
#endif

#endif // remus_common_RemusGlobals_h
