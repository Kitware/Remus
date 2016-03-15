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

namespace  remus {
namespace  common {
namespace  detail {

//------------------------------------------------------------------------------
boost::filesystem::path getOSExecLocation()
{
  //our best guess is the current working directory.
  boost::filesystem::path execLoc = boost::filesystem::current_path();
  return boost::filesystem::canonical(execLoc);
}

}
}
}
