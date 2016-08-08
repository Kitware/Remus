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

#ifndef remus_server_FactoryWorkerSpecification_h
#define remus_server_FactoryWorkerSpecification_h

//included for export symbols
#include <remus/server/ServerExports.h>

#include <remus/proto/JobRequirements.h>

#include <remus/common/CompilerInformation.h>

//force to use filesystem version 3
REMUS_THIRDPARTY_PRE_INCLUDE
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <map>
#include <vector>

namespace remus{
namespace server{

//struct that we use to represent the contents of a worker that the
//factory can launch. Currently the ExtraCommandLineArguments and
//EnvironmentVariables are ignored by the default WorkerFactory, but
//exist to allow for better worker factories designed by users of remus
struct REMUSSERVER_EXPORT FactoryWorkerSpecification
{
  FactoryWorkerSpecification();

  FactoryWorkerSpecification(const boost::filesystem::path& exec_path,
                             const remus::proto::JobRequirements& r);

  FactoryWorkerSpecification(const boost::filesystem::path& exec_path,
                             const std::vector< std::string >& extra_args,
                             const remus::proto::JobRequirements& r);

  FactoryWorkerSpecification(const boost::filesystem::path& exec_path,
                             const std::vector< std::string >& extra_args,
                             const std::map< std::string, std::string >& env,
                             const remus::proto::JobRequirements& r);

  remus::proto::JobRequirements Requirements;
  boost::filesystem::path ExecutionPath;
  std::vector< std::string > ExtraCommandLineArguments;
  std::map< std::string, std::string > EnvironmentVariables;
  bool isValid;
};

}
}

#endif
