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

#include <remus/server/FactoryWorkerSpecification.h>
#include <remus/common/CompilerInformation.h>

//include cjson for parsing the mesh worker file
#include "cJSON.h"

//force to use filesystem version 3
REMUS_THIRDPARTY_PRE_INCLUDE
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

#include <iostream>
#include <fstream>

namespace {
  using namespace remus;
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
  remus::server::FactoryWorkerSpecification parse_json_reqs(
                                        const boost::filesystem::path& file )
  {
    std::string json_contents = read_file(file);
    using namespace remus::common;

    cJSON *root = cJSON_Parse(json_contents.c_str());
    if(!root)
      {
      return remus::server::FactoryWorkerSpecification();
      }

    cJSON *inputT = cJSON_GetObjectItem(root,"InputType");
    cJSON *outputT = cJSON_GetObjectItem(root,"OutputType");
    cJSON *execT = cJSON_GetObjectItem(root,"ExecutableName");
    cJSON *nameT = cJSON_GetObjectItem(root,"WorkerName");
    if(!inputT || !outputT || !execT)
      {
      cJSON_Delete(root);
      return remus::server::FactoryWorkerSpecification();
      }
    const std::string inputType(inputT->valuestring);
    const std::string outputType(outputT->valuestring);
    MeshIOType mesh_type( inputType, outputType );

    //executableName can be a path, so figure it out
    const std::string executableName(execT->valuestring);

    // We need to determine what the name of the worker
    // is. It either is provided explicitly or we need
    // to deduce it from the ExecutableName/Path
    //
    std::string workerName;
    if (nameT && nameT->type == cJSON_String && nameT->valuestring)
      workerName = nameT->valuestring;
    if (workerName.empty())
      {
      boost::filesystem::path execName(executableName);
      workerName = execName.filename().string();
      }

    //by default we select memory and user
    ContentFormat::Type format_type = ContentFormat::User;

    //make the basic memory type reqs
    remus::proto::JobRequirements reqs(format_type,
                                       mesh_type,
                                       workerName,
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
                                             workerName,
                                             rfile);

        }
      }

    // Add the tag string
    cJSON* tagobj = cJSON_GetObjectItem(root, "Tag");
    if (tagobj)
      {
      if (tagobj->type == cJSON_String)
        {
        if (tagobj->valuestring && tagobj->valuestring[0])
          reqs.tag(tagobj->valuestring);
        }
      else
        {
        char* tagstr = cJSON_PrintUnformatted(tagobj);
        reqs.tag(tagstr);
        free(tagstr);
        }
      }

    // Add extra command line arguments
    std::vector< std::string > cmdline;
    cJSON* argobj = cJSON_GetObjectItem(root, "Arguments");
    if (argobj && argobj->type == cJSON_Array)
      {
      cJSON* onearg;
      for (onearg = argobj->child; onearg; onearg = onearg->next)
        if (onearg->type == cJSON_String && onearg->valuestring && onearg->valuestring[0])
          { // Replace the first occurrence of @SELF@ with the path to the worker file.
          std::string strarg(onearg->valuestring);
          std::string::size_type pos = strarg.find("@SELF@");
          if (pos != std::string::npos)
            strarg.replace(pos, 6, file.string());
          cmdline.push_back(strarg);
          }
      }

    // Add environment variables
    std::map< std::string, std::string > environ;
    cJSON* envobj = cJSON_GetObjectItem(root, "Environment");
    if (envobj && envobj->type == cJSON_Object)
      {
      cJSON* oneenv;
      for (oneenv = envobj->child; oneenv; oneenv = oneenv->next)
        if (
          oneenv->type == cJSON_String &&
          oneenv->valuestring &&
          oneenv->valuestring[0] &&
          oneenv->string &&
          oneenv->string[0])
          environ[oneenv->string] = oneenv->valuestring;
      }

    cJSON_Delete(root);

    //try the executableName as an absolute path, also try
    //as an absolute path with .exe suffix if on windows
    boost::filesystem::path mesher_path(executableName);
#ifdef _WIN32
    if(!boost::filesystem::is_regular_file(mesher_path))
      {
      boost::filesystem::path new_path(mesher_path);
      new_path.replace_extension(".exe");
      if(boost::filesystem::is_regular_file(new_path))
        {
        mesher_path = new_path;
        }
      }
#endif

    //handle if we are in a multi config cmake generator location
    //since the provided path would be wrong
#ifdef CMAKE_INTDIR
    if(!boost::filesystem::is_regular_file(mesher_path))
      {
      boost::filesystem::path new_path = mesher_path.parent_path();
      new_path /= std::string(CMAKE_INTDIR);
      new_path /= mesher_path.filename();
#ifdef _WIN32
      new_path.replace_extension(".exe");
#endif
      if (boost::filesystem::is_regular_file(new_path))
        {
        mesher_path = new_path;
        }
      }
#endif

    //fall back to looking based on the file we are parsing,
    //as nothing else worked
    if(!boost::filesystem::is_regular_file(mesher_path))
      {
      boost::filesystem::path new_path(file.parent_path());
      new_path /= mesher_path;
#ifdef _WIN32
      new_path.replace_extension(".exe");
#endif
      mesher_path = new_path;
      }

    return remus::server::FactoryWorkerSpecification(mesher_path, cmdline, environ, reqs);
  }
}


namespace remus{
namespace server{
remus::server::FactoryFileParser::ResultType FactoryFileParser::operator()(
                                   const boost::filesystem::path& file) const
{
  return parse_json_reqs(file);
}


}
}
