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

#include <remus/server/FactoryWorkerSpecification.h>

#ifdef REMUS_MSVC
 #pragma warning(push)
 #pragma warning(disable:4251)  /*dll-interface missing on stl type*/
#endif


namespace remus{
namespace server{

  //----------------------------------------------------------------------------
  FactoryWorkerSpecification::FactoryWorkerSpecification():
    Requirements(),
    ExecutionPath(),
    ExtraCommandLineArguments(),
    EnvironmentVariables(),
    isValid(false)
    {
    }

  //----------------------------------------------------------------------------
  FactoryWorkerSpecification::FactoryWorkerSpecification(
    const boost::filesystem::path& exec_path,
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

  //----------------------------------------------------------------------------
  FactoryWorkerSpecification::FactoryWorkerSpecification(
    const boost::filesystem::path& exec_path,
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

  //----------------------------------------------------------------------------
  FactoryWorkerSpecification::FactoryWorkerSpecification(
    const boost::filesystem::path& exec_path,
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

}
}

#ifdef REMUS_MSVC
  #pragma warning(pop)
#endif
