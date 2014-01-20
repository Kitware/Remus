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

#ifndef _Cubit_Client_client_h
#define _Cubit_Client_client_h

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include "CubitInput.h"
#include "CubitOutput.h"

class client
{
public:
  client(std::string server = "");
  CubitOutput getOutput(CubitInput const& in);
private:
  remus::client::ServerConnection Connection;
};

#endif
