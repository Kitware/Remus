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
     ServiceTypeMacro(TERMINATE_WORKER, 7, "TERMINATE WORKER"), \
     ServiceTypeMacro(CAN_MESH_REQUIREMENTS, 8, "CAN MESH REQUIREMENTS"), \
     ServiceTypeMacro(MESH_REQUIREMENTS, 9, "MESH REQUIREMENTS")

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
  for(int i=1; i <=9; i++)
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
