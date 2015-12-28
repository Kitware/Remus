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

#ifndef remus_server_PortNumbers_h
#define remus_server_PortNumbers_h

namespace remus {
namespace server {
//remus::server::CLIENT_PORT is the port that clients connect on, to
//submit and query about jobs
static const int CLIENT_PORT = 50505;

//remus::server::WORKER_PORT is used by workers to fetch jobs,
//and send back the status and result of the job
//Workers also use the remus::server::WORKER_PORT for heart beating
static const int WORKER_PORT = 50510;

//remus::server::SERVER_STATUS_PORT is the port the server publishes all forms of async
//notifications onto. Clients, Loggers, and Monitors can connect to this
//port to watch the status of jobs, see the health of the server, etc.
static const int STATUS_PORT = 50550;

}
}

#endif