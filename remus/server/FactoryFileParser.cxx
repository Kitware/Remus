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

#include <remus/server/FactoryFileParser.h>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem/fstream.hpp>

//include cjson for parsing the mesh worker file
#include "cJSON.h"

#include <iostream>

//for make_pair
#include <utility>

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


namespace remus{
namespace server{
FactoryFileParser::ResultType FactoryFileParser::operator()(
                               const boost::filesystem::path& file) const
{
  return parse_json_reqs(file);
}


}
}
