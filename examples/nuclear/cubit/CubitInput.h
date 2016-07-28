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

#ifndef _CubitInput_h
#define _CubitInput_h

#include <string>
#include <iostream>
#include <sstream>

#include <remus/worker/Job.h>
#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/algorithm/string.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

class CubitInput
{
public:
  CubitInput(const remus::worker::Job& job)
  {
    std::stringstream ss(job.details("default"));
    getline(ss, InFile, ';');
    boost::algorithm::trim_if(InFile,boost::algorithm::is_cntrl());
    getline(ss, ExecutablePath, ';');
    boost::algorithm::trim_if(ExecutablePath, boost::algorithm::is_cntrl());
  }
  CubitInput(std::string& inFile, std::string & path)
    : InFile(inFile), ExecutablePath(path)
  {
  }
  std::string const& getInputFile() const
  {
    return InFile;
  }
  void setInputFile(std::string p)
  { InFile = p; }

  std::string const& getExecutablePath() const
  { return ExecutablePath; }

  operator std::string(void) const
  {
    std::stringstream ss;
    ss << InFile << ";" <<ExecutablePath << ";";
    return ss.str();
  }
private:
  std::string InFile;
  std::string ExecutablePath;
};

#endif
