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
  remus::proto::JobRequirements Requirements;
  boost::filesystem::path ExecutionPath;
  std::vector< std::string > ExtraCommandLineArguments;
  std::map< std::string, std::string > EnvironmentVariables;
  bool isValid;

  FactoryWorkerSpecification():
    Requirements(),
    ExecutionPath(),
    ExtraCommandLineArguments(),
    EnvironmentVariables(),
    isValid(false)
    {
    }

  FactoryWorkerSpecification(const boost::filesystem::path& exec_path,
                             const remus::proto::JobRequirements& r):
    Requirements(r),
    ExecutionPath(),
    ExtraCommandLineArguments(),
    EnvironmentVariables(),
    isValid(false)
    {
    if(boost::filesystem::is_regular_file(exec_path))
      {
      //convert the exec_path into an absolute canonical path now
      this->ExecutionPath = boost::filesystem::canonical(exec_path);
      this->isValid = true;
      }
    }

  FactoryWorkerSpecification(const boost::filesystem::path& exec_path,
                             const std::vector< std::string >& extra_args,
                             const remus::proto::JobRequirements& r):
    Requirements(r),
    ExecutionPath(),
    ExtraCommandLineArguments(extra_args),
    EnvironmentVariables(),
    isValid(false)
    {
    if(boost::filesystem::is_regular_file(exec_path))
      {
      //convert the exec_path into an absolute canonical path now
      this->ExecutionPath = boost::filesystem::canonical(exec_path);
      this->isValid = true;
      }
    }

  FactoryWorkerSpecification(const boost::filesystem::path& exec_path,
                             const std::vector< std::string >& extra_args,
                             const std::map< std::string, std::string >& environment,
                             const remus::proto::JobRequirements& r):
    Requirements(r),
    ExecutionPath(),
    ExtraCommandLineArguments(extra_args),
    EnvironmentVariables(environment),
    isValid(false)
    {
    if(boost::filesystem::is_regular_file(exec_path))
      {
      //convert the exec_path into an absolute canonical path now
      this->ExecutionPath = boost::filesystem::canonical(exec_path);
      this->isValid = true;
      }
    }
};

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
