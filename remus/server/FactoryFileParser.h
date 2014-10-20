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

#include <remus/proto/JobRequirements.h>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

namespace remus{
namespace server{
//Extensible class that determines how to parse the contents of a WorkerFactory
//input file. Can
class REMUSSERVER_EXPORT FactoryFileParser
{
public:
  typedef std::pair< boost::filesystem::path, remus::proto::JobRequirements > ResultType;

  virtual ResultType operator()( const boost::filesystem::path& file ) const;
};

}
}
#endif