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

#ifndef _AssygenInput_h
#define _AssygenInput_h

#include <string>
#include <iostream>
#include <sstream>

#include <remus/worker/Job.h>

#include <remus/common/CompilerInformation.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/algorithm/string.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

class AssygenInput
{
public:
  AssygenInput(const remus::worker::Job& job)
  {
    std::stringstream ss(job.details("default"));
    getline(ss, Prefix, ';');
    boost::algorithm::trim_if(Prefix,boost::algorithm::is_cntrl());
    getline(ss, ExecutablePath, ';');
    boost::algorithm::trim_if(ExecutablePath, boost::algorithm::is_cntrl());
  }
  AssygenInput(std::string& prefix, std::string & path)
    : Prefix(prefix), ExecutablePath(path)
  {
  }
  std::string const& getPrefix() const
  {
    return Prefix;
  }
  void setPrefix(std::string p)
  { Prefix = p; }

  std::string const& getExecutablePath() const
  { return ExecutablePath; }

  operator std::string(void) const
  {
    std::stringstream ss;
    ss << Prefix << ";" <<ExecutablePath << ";";
    return ss.str();
  }
private:
  std::string Prefix;
  std::string ExecutablePath;
};

#endif
