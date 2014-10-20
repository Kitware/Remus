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

#include <remus/server/detail/WorkerFinder.h>

#include <remus/common/MeshRegistrar.h>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <iostream>

//include cjson for parsing the mesh worker file
#include "cJSON.h"

//for make_pair
#include <utility>

namespace remus{
namespace server{
namespace detail{


namespace {
  //----------------------------------------------------------------------------
  std::string read_file(const boost::filesystem::path& file)
  {
    std::string file_contents;
    std::string path = boost::filesystem::canonical(file).string();
    std::ifstream f(path.c_str(),
                    std::ios_base::in | std::ios::binary);
    if(f.is_open())
      {
      //read the file into a std::string
      f.seekg(0, std::ios::end);
      file_contents.resize( f.tellg() );
      f.seekg(0, std::ios::beg);
      f.read(&file_contents[0], file_contents.size());
      }
    f.close();
    return file_contents;
  }

  //----------------------------------------------------------------------------
  std::pair< boost::filesystem::path, remus::proto::JobRequirements >
  parse_json_reqs( const boost::filesystem::path& file )
  {
    std::string json_contents = read_file(file);
    using namespace remus::common;

    cJSON *root = cJSON_Parse(json_contents.c_str());
    if(!root)
      {
      return std::make_pair( boost::filesystem::path(),
                             remus::proto::JobRequirements() );
      }

    cJSON *inputT = cJSON_GetObjectItem(root,"InputType");
    cJSON *outputT = cJSON_GetObjectItem(root,"OutputType");
    cJSON *execT = cJSON_GetObjectItem(root,"ExecutableName");
    if(!inputT || !outputT || !execT)
      {
      cJSON_Delete(root);
      return std::make_pair( boost::filesystem::path(),
                             remus::proto::JobRequirements() );
      }
    const std::string inputType(inputT->valuestring);
    const std::string outputType(outputT->valuestring);
    MeshIOType mesh_type( inputType, outputType );

    //executableName can be a path, so figure it out
    std::string executableName(execT->valuestring);
    boost::filesystem::path mesher_path(executableName);
    if(boost::filesystem::is_regular_file(mesher_path))
      {
      executableName = mesher_path.filename().string();
      }

    //by default we select memory and user
    ContentFormat::Type format_type = ContentFormat::User;

    //make the basic memory type reqs
    remus::proto::JobRequirements reqs(format_type,
                                       mesh_type,
                                       executableName,
                                       std::string());

    //check if we have a specific file requirements
    cJSON *req_file = cJSON_GetObjectItem(root,"File");
    cJSON *req_file_format = cJSON_GetObjectItem(root,"FileFormat");
    if(req_file)
      {
      //we have a file source type, now determine the format type, and
      //read in the data from the file
      if(req_file_format)
        {
        if(strncmp(req_file_format->valuestring,"XML",3) == 0)
          { format_type = ContentFormat::XML; }
        else if(strncmp(req_file_format->valuestring,"JSON",4) == 0)
          { format_type = ContentFormat::JSON; }
        else if(strncmp(req_file_format->valuestring,"BSON",4) == 0)
          { format_type = ContentFormat::BSON; }
        }

      //now try to read the file
      boost::filesystem::path req_file_path(req_file->valuestring);
      if(!boost::filesystem::is_regular_file(req_file_path))
        {
        req_file_path = boost::filesystem::path(file.parent_path());
        req_file_path /= req_file->valuestring;
        }

      if(boost::filesystem::is_regular_file(req_file_path))
        {
        //load the contents of the file into a string to be given to the
        //requirements
        remus::common::FileHandle rfile(
                        boost::filesystem::canonical(req_file_path).string());
        reqs = remus::proto::JobRequirements(format_type,
                                             mesh_type,
                                             executableName,
                                             rfile);

        }
      }
    cJSON_Delete(root);

    return std::make_pair( mesher_path, reqs );
  }
}

//class that loads all files in the executing directory
//with the msw extension and creates a vector of possible
//meshers with that info
//----------------------------------------------------------------------------
WorkerFinder::WorkerFinder(const std::string& ext):
  FileExt( boost::algorithm::to_upper_copy(ext) ),
  Info()
{
  boost::filesystem::path cwd = boost::filesystem::current_path();
  this->parseDirectory(cwd);
}

//----------------------------------------------------------------------------
WorkerFinder::WorkerFinder(const boost::filesystem::path& path,
                           const std::string& ext):
  FileExt( boost::algorithm::to_upper_copy(ext) ),
  Info()
{
  this->parseDirectory(path);
}

//----------------------------------------------------------------------------
void WorkerFinder::parseDirectory(const boost::filesystem::path& dir)
  {
  if( boost::filesystem::is_directory(dir) )
    {
    boost::filesystem::directory_iterator end_itr;
    for( boost::filesystem::directory_iterator i( dir ); i != end_itr; ++i )
      {
      // Skip if not a file
      if(boost::filesystem::is_regular_file( i->status() ))
        {
        const boost::filesystem::path ipath = i->path();
        if(ipath.has_extension())
          {
          const std::string ext =
                    boost::algorithm::to_upper_copy(ipath.extension().string());
          if(ext == FileExt)
            {
            this->parseFile(i->path());
            }
          }
        }
      }
    }
  }

//----------------------------------------------------------------------------
void WorkerFinder::parseFile(const boost::filesystem::path& file )
{
  typedef std::pair< boost::filesystem::path,
                     remus::proto::JobRequirements > ReturnType;
  ReturnType info = parse_json_reqs( file );

  //try the executableName as an absolute path, if that isn't
  //a file than fall back to looking based on the file we are parsing
  //path
  boost::filesystem::path mesher_path(info.first);
  if(!boost::filesystem::is_regular_file(mesher_path))
    {
    boost::filesystem::path new_path(file.parent_path());
    new_path /= info.first;
#ifdef _WIN32
    new_path.replace_extension(".exe");
#endif
    mesher_path = new_path;
    }

  if(boost::filesystem::is_regular_file(mesher_path))
    {
    //convert the mesher_path into an absolute canonical path now
    mesher_path = boost::filesystem::canonical(mesher_path);
    this->Info.push_back(MeshWorkerInfo(info.second,mesher_path.string()));
    }
}

}
}
}