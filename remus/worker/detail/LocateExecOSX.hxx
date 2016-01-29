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

#include <CoreFoundation/CoreFoundation.h>

namespace  remus {
namespace  worker {

//------------------------------------------------------------------------------
boost::filesystem::path getOSExecLocation()
{
  boost::filesystem::path execLoc;
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  if(mainBundle)
    {
    CFURLRef url (CFBundleCopyExecutableURL(mainBundle));
    if(url)
      {
      CFStringRef posixUrl (CFURLCopyFileSystemPath(url,kCFURLPOSIXPathStyle));
      if(posixUrl)
        {
        int len = static_cast<int>(CFStringGetLength(posixUrl));
        int bufferSize = static_cast<int>(CFStringGetMaximumSizeForEncoding(len,
                                          kCFStringEncodingUTF8));
        if(len > 0 && bufferSize > 0)
          {
          //don't use len, that returns the len for utf16, which isn't accurate
          //you need to get the maxium size for the c string encoding
          char* name = new char[bufferSize+1];
          bool nameCopied = CFStringGetCString(posixUrl,name,bufferSize,
                                               kCFStringEncodingUTF8);
          if(nameCopied)
            {
            //we have a valid bundle path, copy to string and drop the executables name.
            const std::string execFileName(name);
            const boost::filesystem::path execFile = boost::filesystem::path(execFileName);
            execLoc = execFile.parent_path();
            execLoc = boost::filesystem::canonical(execLoc);
            }
          delete[] name;
          }
        CFRelease(posixUrl);
        }
      CFRelease(url);
      }
    }
  return execLoc;
}

}
}
