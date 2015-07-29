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

#ifndef remus_proto_EventPublisher_h
#define remus_proto_EventPublisher_h

#include <string>

namespace remus {
namespace proto {
namespace jobevents {

#define JobEventTypeMacros() \
     JobEventTypeMacro(INVALID, 0,"INVALID"), \
     JobEventTypeMacro(QUEUED, 1,"QUEUED"), \
     JobEventTypeMacro(JOB_STATUS, 2, "CURRENT JOB STATUS"), \
     JobEventTypeMacro(TERMINATED, 3, "TERMINATED"), \
     JobEventTypeMacro(EXPIRED, 4, "EXPIRED"), \
     JobEventTypeMacro(COMPLETED, 5, "COMPLETED")

//------------------------------------------------------------------------------
enum EVENT_TYPE
{
#define JobEventTypeMacro(ID,NUM,NAME) ID = NUM
   JobEventTypeMacros()
#undef JobEventTypeMacro
};

//------------------------------------------------------------------------------
//a mapping of enum types to char*
static const char *event_types[] = {
#define JobEventTypeMacro(ID,NUM,NAME) NAME
  JobEventTypeMacros()
#undef JobEventTypeMacro
};

//------------------------------------------------------------------------------
inline std::string to_string(remus::proto::jobevents::EVENT_TYPE ev)
{
  return std::string(remus::proto::jobevents::event_types[(int)ev]);
}
}

namespace workevents {

#define WorkEventTypeMacros() \
     WorkEventTypeMacro(INVALID, 0,"INVALID"), \
     WorkEventTypeMacro(NOT_USED_1, 1, "NOT USED"), \
     WorkEventTypeMacro(JOB_STATUS, 2, "CURRENT JOB STATUS"), \
     WorkEventTypeMacro(TERMINATED, 3, "TERMINATED"), \
     WorkEventTypeMacro(NOT_USED_4, 4, "NOT USED"), \
     WorkEventTypeMacro(COMPLETED, 5, "COMPLETED"), \
     WorkEventTypeMacro(ASSIGNED_TO_WORKER, 6, "ASSIGNED TO WORKER"), \
     WorkEventTypeMacro(REGISTERED, 7,"REGISTERED"), \
     WorkEventTypeMacro(ASKING_FOR_JOB, 8, "ASKING_FOR_JOB"), \
     WorkEventTypeMacro(HEARTBEAT, 9, "HEARTBEAT"), \
     WorkEventTypeMacro(WORKER_STATE, 10, "WORKER STATE"), \
     WorkEventTypeMacro(TERMINATED_WORKER, 11, "TERMINATED WORKER")

//------------------------------------------------------------------------------
enum EVENT_TYPE
{
#define WorkEventTypeMacro(ID,NUM,NAME) ID = NUM
   WorkEventTypeMacros()
#undef WorkEventTypeMacro
};

//------------------------------------------------------------------------------
//a mapping of enum types to char*
static const char *event_types[] = {
#define WorkEventTypeMacro(ID,NUM,NAME) NAME
  WorkEventTypeMacros()
#undef WorkEventTypeMacro
};

//------------------------------------------------------------------------------
inline std::string to_string(remus::proto::workevents::EVENT_TYPE ev)
{
  return std::string(remus::proto::workevents::event_types[(int)ev]);
}
}


}
}

#endif //remus_proto_EventPublisher_h
