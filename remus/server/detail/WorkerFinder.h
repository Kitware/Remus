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

#ifndef remus_server_detail_WorkerFinder_h
#define remus_server_detail_WorkerFinder_h

#include <remus/proto/JobRequirements.h>

#include <vector>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

namespace remus{
namespace server{
namespace detail{

//struct that we use to represent the contents of a worker that the
//factory can launch
struct MeshWorkerInfo
{
  remus::proto::JobRequirements Requirements;
  std::string ExecutionPath;
  MeshWorkerInfo(const remus::proto::JobRequirements& r,
                 const std::string& p):
    Requirements(r),ExecutionPath(p){}
};

//Helper class that is used by the WorkerFactory to locate and parse
//Worker json files
class WorkerFinder
{
  //class that loads all files in the executing directory
  //with the rw extension and creates a vector of possible
  //meshers with that info
public:
  typedef std::vector<MeshWorkerInfo>::const_iterator const_iterator;
  typedef std::vector<MeshWorkerInfo>::iterator iterator;

  WorkerFinder(const std::string& ext);

  WorkerFinder(const boost::filesystem::path& path,
               const std::string& ext);

  void parseDirectory(const boost::filesystem::path& dir);

  void parseFile(const boost::filesystem::path& file);

  const std::vector<MeshWorkerInfo>& results() const {return Info;}

  iterator begin() {return this->Info.begin();}
  const_iterator begin() const {return this->Info.begin();}

  iterator end() {return this->Info.end();}
  const_iterator end() const {return this->Info.end();}

private:
  const std::string FileExt;
  std::vector<MeshWorkerInfo> Info;
};

}
}
}

#endif