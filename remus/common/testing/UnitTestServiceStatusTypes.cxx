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

#include <string>

#include <remus/common/ServiceTypes.h>
#include <remus/common/StatusTypes.h>

#include <remus/testing/Testing.h>


int UnitTestServiceStatusTypes(int, char *[])
{
  //verify all service types
 for(int i=1; i <=9; i++)
    {
    remus::SERVICE_TYPE mt=static_cast<remus::SERVICE_TYPE>(i);
    std::string service_str = remus::to_string(mt);
    REMUS_ASSERT( (remus::common::serv_types[i] == service_str) );
    remus::SERVICE_TYPE other = remus::to_serviceType(service_str);
    REMUS_ASSERT( (other == mt) );
    }

  //verify all status types
  for(int i=1; i <=5; i++)
    {
    remus::STATUS_TYPE mt=static_cast<remus::STATUS_TYPE>(i);
    std::string status_str = remus::to_string(mt);
    REMUS_ASSERT( (remus::common::stat_types[i] == status_str) );
    remus::STATUS_TYPE other = remus::to_statusType(status_str);
    REMUS_ASSERT( (other == mt) );
    }

  //verify converting an invalid string gets us back the invalid flag
  remus::SERVICE_TYPE invalid_service = remus::to_serviceType("FOO");
  remus::STATUS_TYPE invalid_status = remus::to_statusType("FOO");

  REMUS_ASSERT( (invalid_service == remus::INVALID_SERVICE) );
  REMUS_ASSERT( (invalid_status == remus::INVALID_STATUS) );


  return 0;
}
