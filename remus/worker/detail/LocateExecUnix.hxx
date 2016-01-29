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

#include <sys/auxv.h>
#include <unistd.h>

namespace  remus {
namespace  worker {

//------------------------------------------------------------------------------
boost::filesystem::path getOSExecLocation()
{
  boost::filesystem::path execLoc;

  //On linux we can use the glibc auxiliary vector to get the invoked path
  //to the executable. This could be a relative or absolute path
  const char *executedPath = (char *)getauxval(AT_EXECFN);
  if(!executedPath)
    {
    return execLoc;
    }

  //drop the executable name
  boost::filesystem::path execPath(executedPath);
  execPath = execPath.parent_path();

  //determine if we are an absolute or relative path
  if(execPath.is_absolute())
    {
    return execPath;
    }
  else
    {
    //convert the relative path to an absolute one
    execLoc = boost::filesystem::current_path();
    execLoc /= execPath;
    return boost::filesystem::canonical(execLoc);
    }

}

}
}
