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

#ifndef _Coregen_Client_client_h
#define _Coregen_Client_client_h

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include "CoregenInput.h"
#include "CoregenOutput.h"

class client
{
public:
  client(std::string server = "");
  CoregenOutput getOutput(CoregenInput const& in);
private:
  remus::client::ServerConnection Connection;
};

#endif
