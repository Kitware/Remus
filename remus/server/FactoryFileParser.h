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

#ifndef remus_server_FactoryFileParser_h
#define remus_server_FactoryFileParser_h

//included for export symbols
#include <remus/server/ServerExports.h>

namespace boost{
namespace filesystem{
class path;
}
}

namespace remus{
namespace server{

struct FactoryWorkerSpecification;

//Extensible class that determines how to parse the contents of a WorkerFactory
//input file. Can
class REMUSSERVER_EXPORT FactoryFileParser
{
public:
  typedef FactoryWorkerSpecification ResultType;

  virtual ResultType operator()( const boost::filesystem::path& file ) const;
};

}
}

#endif
