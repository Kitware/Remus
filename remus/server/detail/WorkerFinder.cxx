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
#include <remus/server/FactoryFileParser.h>

#include <remus/common/MeshRegistrar.h>

REMUS_THIRDPARTY_PRE_INCLUDE
#include <boost/algorithm/string/case_conv.hpp>
REMUS_THIRDPARTY_POST_INCLUDE

namespace remus{
namespace server{
namespace detail{

//class that loads all files in the executing directory
//with the msw extension and creates a vector of possible
//meshers with that info
//----------------------------------------------------------------------------
WorkerFinder::WorkerFinder(const FactoryFileParserPtr& parser,
                           const std::string& ext):
  FileExt( boost::algorithm::to_upper_copy(ext) ),
  Parser(parser),
  Info()
{
  boost::filesystem::path cwd = boost::filesystem::current_path();
  this->parseDirectory(cwd);
}

//----------------------------------------------------------------------------
WorkerFinder::WorkerFinder(const FactoryFileParserPtr& parser,
                           const boost::filesystem::path& path,
                           const std::string& ext):
  FileExt( boost::algorithm::to_upper_copy(ext) ),
  Parser(parser),
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
  typedef remus::server::FactoryFileParser::ResultType ReturnType;
  ReturnType info =(*this->Parser)( file );
  if( info.isValid )
    {
    this->Info.push_back(info);
    }
}

}
}
}
